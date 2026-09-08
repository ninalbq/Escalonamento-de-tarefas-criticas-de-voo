# Escalonamento de Tarefas Críticas de Voo

Simulador de escalonamento preemptivo para tarefas periódicas com deadlines menores que o período. Compara dois algoritmos clássicos de tempo real:

- prioridade fixa (menor período → maior prioridade).
- prioridade dinâmica (menor deadline absoluto → maior prioridade).

## Compilação

```bash
gcc -o scheduler scheduler.c
 
``` 
#execucao

./scheduler rate voo.txt ou ./scheduler rate voo2.txt ou como preferir

  Gera o arquivo de saída rate_ncao2.out ou edf_ncao2.out

 Formato de entrada
text
tempo_total
nome periodo deadline burst

Todos os valores são inteiros positivos.

Deve valer C ≤ D ≤ P

O arquivo de saída contém:

EXECUTION BY...: intervalos de execução com flags (F = finalizada, H = preemptada, L = perdeu deadline, K = morta pelo fim).

LOST DEADLINES: quantas instâncias perderam o prazo

COMPLETE EXECUTION: quantas completaram

KILLED: quantas foram mortas até fim da simulação.

