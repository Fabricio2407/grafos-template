#include <stdio.h>
#include <stdlib.h>

#include "dag.h"

static void mostrar_ordenacao(const char *algoritmo, int *ordem, int tamanho) {
    printf("%s:\n", algoritmo);
    if (ordem == NULL) {
        printf("Ordenacao impossivel: grafo possui ciclo.\n");
        return;
    }
    for (int i = 0; i < tamanho; i++) {
        printf("%d%s", ordem[i], i + 1 == tamanho ? "\n" : " ");
    }
}

static void demonstrar_grafo(const char *nome, GrafoLista *g) {
    int tamanho_kahn;
    int tamanho_dfs;
    int *ordem_kahn = ordenacao_topologica_kahn(g, &tamanho_kahn);
    int *ordem_dfs = ordenacao_topologica_dfs(g, &tamanho_dfs);

    printf("\n=== %s ===\n\n", nome);
    printf("Eh DAG: %s\n\n", eh_dag(g) ? "sim" : "nao");
    mostrar_ordenacao("Ordenacao topologica - Kahn", ordem_kahn, tamanho_kahn);
    printf("\n");
    mostrar_ordenacao("Ordenacao topologica - DFS", ordem_dfs, tamanho_dfs);

    free(ordem_kahn);
    free(ordem_dfs);
}

int main(void) {
    GrafoLista *dag = criar_grafo_lista(6);
    GrafoLista *ciclico = criar_grafo_lista(3);
    if (dag == NULL || ciclico == NULL) {
        liberar_grafo_lista(dag);
        liberar_grafo_lista(ciclico);
        return 1;
    }

    inserir_aresta_direcionada(dag, 5, 2);
    inserir_aresta_direcionada(dag, 5, 0);
    inserir_aresta_direcionada(dag, 4, 0);
    inserir_aresta_direcionada(dag, 4, 1);
    inserir_aresta_direcionada(dag, 2, 3);
    inserir_aresta_direcionada(dag, 3, 1);

    inserir_aresta_direcionada(ciclico, 0, 1);
    inserir_aresta_direcionada(ciclico, 1, 2);
    inserir_aresta_direcionada(ciclico, 2, 0);

    demonstrar_grafo("GRAFO 1: DAG", dag);
    demonstrar_grafo("GRAFO 2: COM CICLO", ciclico);

    liberar_grafo_lista(dag);
    liberar_grafo_lista(ciclico);
    return 0;
}
