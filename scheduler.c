#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOGIN "ncao2"

typedef struct {
    char nome[64];
    int periodo;
    int deadline;
    int burst;
    int ordem;
} Tarefa;

typedef struct Instancia {
    int indice_tarefa;
    int restante;
    int deadline_abs;
    struct Instancia *prox; // não usado, reservado
} Instancia;

static int tempo_total;
static int num_tarefas;
static Tarefa *tarefas;
static Instancia **ativas; // uma instância ativa por tarefa

static int *executado;     // executado[t] = índice da tarefa que rodou em [t, t+1)
static int *completou_em;  // completou_em[t] = tarefa que terminou no instante t
static int *perdeu_em;     // perdeu_em[t] = tarefa que estourou deadline no instante t

static int *perdidas;
static int *completadas;
static int *mortas;

static void erro(const char *msg) {
    fprintf(stderr, "Erro: %s\n", msg);
    exit(EXIT_FAILURE);
}

// Retorna 1 se a tarefa 'a' tem prioridade sobre 'b' no algoritmo 'algo'
static int maior_prioridade(int a, int b, char algo) {
    if (algo == 'R') { // Rate-Monotonic: menor período tem prioridade
        if (tarefas[a].periodo < tarefas[b].periodo) return 1;
        if (tarefas[a].periodo > tarefas[b].periodo) return 0;
        return a < b; // desempate pela ordem de declaração
    } else {           // EDF: menor deadline absoluto tem prioridade
        if (ativas[a]->deadline_abs < ativas[b]->deadline_abs) return 1;
        if (ativas[a]->deadline_abs > ativas[b]->deadline_abs) return 0;
        return a < b;
    }
}

static void ler_entrada(const char *arquivo) {
    FILE *fp = fopen(arquivo, "r");
    if (!fp) erro("arquivo de entrada inexistente ou ilegível");

    char linha[256];
    if (fgets(linha, sizeof(linha), fp) == NULL)
        erro("arquivo vazio");

    if (sscanf(linha, "%d", &tempo_total) != 1 || tempo_total <= 0)
        erro("tempo total inválido");

    // conta quantas tarefas existem
    int cont = 0;
    long posicao = ftell(fp);
    while (fgets(linha, sizeof(linha), fp)) {
        if (linha[0] == '\n' || linha[0] == '\r') continue;
        char nome[64];
        int p, d, c;
        if (sscanf(linha, "%s %d %d %d", nome, &p, &d, &c) != 4)
            erro("linha de tarefa malformada");
        if (p <= 0 || d <= 0 || c <= 0)
            erro("valores devem ser positivos");
        if (d > p || c > d)
            erro("violação de C ≤ D ≤ P");
        cont++;
    }
    if (cont == 0) erro("nenhuma tarefa definida");

    num_tarefas = cont;
    tarefas = malloc(num_tarefas * sizeof(Tarefa));
    ativas = calloc(num_tarefas, sizeof(Instancia*));
    perdidas = calloc(num_tarefas, sizeof(int));
    completadas = calloc(num_tarefas, sizeof(int));
    mortas = calloc(num_tarefas, sizeof(int));

    // volta ao início das tarefas e carrega os dados
    fseek(fp, posicao, SEEK_SET);
    int i = 0;
    while (fgets(linha, sizeof(linha), fp)) {
        if (linha[0] == '\n' || linha[0] == '\r') continue;
        char nome[64];
        int p, d, c;
        sscanf(linha, "%s %d %d %d", nome, &p, &d, &c);
        strcpy(tarefas[i].nome, nome);
        tarefas[i].periodo = p;
        tarefas[i].deadline = d;
        tarefas[i].burst = c;
        tarefas[i].ordem = i;
        i++;
    }
    fclose(fp);
}

static void simular(char algo) {
    executado = malloc(tempo_total * sizeof(int));
    completou_em = malloc((tempo_total + 1) * sizeof(int));
    perdeu_em = malloc((tempo_total + 1) * sizeof(int));
    for (int t = 0; t < tempo_total; t++) executado[t] = -1;
    for (int t = 0; t <= tempo_total; t++) {
        completou_em[t] = -1;
        perdeu_em[t] = -1;
    }

    for (int t = 0; t < tempo_total; t++) {
        // 1. Trata deadlines que vencem exatamente em t
        for (int i = 0; i < num_tarefas; i++) {
            if (ativas[i] != NULL && ativas[i]->deadline_abs == t) {
                perdeu_em[t] = i;
                perdidas[i]++;
                free(ativas[i]);
                ativas[i] = NULL;
            }
        }

        // 2. Chegada de novas instâncias
        for (int i = 0; i < num_tarefas; i++) {
            if (t % tarefas[i].periodo == 0) {
                if (ativas[i] != NULL) {
                    // Se já havia uma ativa (não deveria ocorrer com D ≤ P), mata a antiga
                    mortas[i]++;
                    free(ativas[i]);
                    ativas[i] = NULL;
                }
                Instancia *inst = malloc(sizeof(Instancia));
                inst->indice_tarefa = i;
                inst->restante = tarefas[i].burst;
                inst->deadline_abs = t + tarefas[i].deadline;
                inst->prox = NULL;
                ativas[i] = inst;
            }
        }

        // 3. Escolhe a tarefa de maior prioridade
        int atual = -1;
        for (int i = 0; i < num_tarefas; i++) {
            if (ativas[i] == NULL) continue;
            if (atual == -1 || maior_prioridade(i, atual, algo))
                atual = i;
        }

        // 4. Executa uma unidade de tempo
        if (atual != -1) {
            executado[t] = atual;
            ativas[atual]->restante--;
            if (ativas[atual]->restante == 0) {
                completou_em[t + 1] = atual;
                completadas[atual]++;
                free(ativas[atual]);
                ativas[atual] = NULL;
            }
        }
    }

    // Verifica deadlines que vencem exatamente no fim da simulação
    for (int i = 0; i < num_tarefas; i++) {
        if (ativas[i] != NULL && ativas[i]->deadline_abs == tempo_total) {
            perdeu_em[tempo_total] = i;
            perdidas[i]++;
            free(ativas[i]);
            ativas[i] = NULL;
        }
    }

    // Instâncias ainda ativas são "mortas" pelo fim da simulação
    for (int i = 0; i < num_tarefas; i++) {
        if (ativas[i] != NULL) {
            mortas[i]++;
            free(ativas[i]);
            ativas[i] = NULL;
        }
    }
}
