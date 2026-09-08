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
    struct Instancia *prox;
} Instancia;

static int tempo_total;
static int num_tarefas;
static Tarefa *tarefas;
static Instancia **ativas;

static int *executado;
static int *completou_em;
static int *perdeu_em;

static int *perdidas;
static int *completadas;
static int *mortas;

static void erro(const char *msg) {
    fprintf(stderr, "Erro: %s\n", msg);
    exit(EXIT_FAILURE);
}

static int maior_prioridade(int a, int b, char algo) {
    if (algo == 'R') {
        if (tarefas[a].periodo < tarefas[b].periodo) return 1;
        if (tarefas[a].periodo > tarefas[b].periodo) return 0;
        return a < b;
    } else {
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
        for (int i = 0; i < num_tarefas; i++) {
            if (ativas[i] != NULL && ativas[i]->deadline_abs == t) {
                perdeu_em[t] = i;
                perdidas[i]++;
                free(ativas[i]);
                ativas[i] = NULL;
            }
        }

        for (int i = 0; i < num_tarefas; i++) {
            if (t % tarefas[i].periodo == 0) {
                if (ativas[i] != NULL) {
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

        int atual = -1;
        for (int i = 0; i < num_tarefas; i++) {
            if (ativas[i] == NULL) continue;
            if (atual == -1 || maior_prioridade(i, atual, algo))
                atual = i;
        }

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

    for (int i = 0; i < num_tarefas; i++) {
        if (ativas[i] != NULL && ativas[i]->deadline_abs == tempo_total) {
            perdeu_em[tempo_total] = i;
            perdidas[i]++;
            free(ativas[i]);
            ativas[i] = NULL;
        }
    }

    for (int i = 0; i < num_tarefas; i++) {
        if (ativas[i] != NULL) {
            mortas[i]++;
            free(ativas[i]);
            ativas[i] = NULL;
        }
    }
}

static void gerar_saida(char algo) {
    char nome_arquivo[128];
    snprintf(nome_arquivo, sizeof(nome_arquivo), "%s_%s.out",
             (algo == 'R') ? "rate" : "edf", LOGIN);

    FILE *out = fopen(nome_arquivo, "w");
    if (!out) erro("não foi possível criar arquivo de saída");

    fprintf(out, "EXECUTION BY %s\n", (algo == 'R') ? "RATE" : "EDF");

    int idx = 0;
    while (idx < tempo_total) {
        int tarefa = executado[idx];
        int comprimento = 1;
        while (idx + comprimento < tempo_total && executado[idx + comprimento] == tarefa)
            comprimento++;

        if (tarefa == -1) {
            fprintf(out, "idle for %d units\n", comprimento);
        } else {
            int fim = idx + comprimento;
char flag;
    if (completou_em[fim] == tarefa) {
    flag = 'F';
}   
else if (perdeu_em[fim] == tarefa) {
    flag = 'L';
} 
else if (fim == tempo_total) {
    flag = 'K';
} else {
    flag = 'H';
}
fprintf(out, "[%s] for %d units - %c\n",
    tarefas[tarefa].nome, comprimento, flag);
            
        }
        idx += comprimento;
    }

    fprintf(out, "\nLOST DEADLINES\n");
    for (int i = 0; i < num_tarefas; i++)
        fprintf(out, "[%s] %d\n", tarefas[i].nome, perdidas[i]);

    fprintf(out, "\nCOMPLETE EXECUTION\n");
    for (int i = 0; i < num_tarefas; i++)
        fprintf(out, "[%s] %d\n", tarefas[i].nome, completadas[i]);

    fprintf(out, "\nKILLED\n");
    for (int i = 0; i < num_tarefas; i++)
        fprintf(out, "[%s] %d\n", tarefas[i].nome, mortas[i]);

    fclose(out);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <rate|edf> <arquivo>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char algo;
    if (strcmp(argv[1], "rate") == 0)
        algo = 'R';
    else if (strcmp(argv[1], "edf") == 0)
        algo = 'E';
    else {
        fprintf(stderr, "Erro: primeiro argumento deve ser 'rate' ou 'edf'\n");
        return EXIT_FAILURE;
    }

    ler_entrada(argv[2]);
    simular(algo);
    gerar_saida(algo);

    free(tarefas);
    free(ativas);
    free(executado);
    free(completou_em);
    free(perdeu_em);
    free(perdidas);
    free(completadas);
    free(mortas);

    return EXIT_SUCCESS;
}