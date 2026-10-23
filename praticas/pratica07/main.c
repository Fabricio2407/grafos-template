#include <stdlib.h>

#include "bellman_ford.h"

static int demonstrar(const char *nome, int n, const int arestas[][3],
                      int quantidade) {
    GrafoListaPonderado *g = criar_grafo_ponderado(n);
    int *dist = malloc((size_t)n * sizeof(*dist));
    int *pred = malloc((size_t)n * sizeof(*pred));
    int resultado = EXIT_FAILURE;
    if (g == NULL || dist == NULL || pred == NULL) {
        fprintf(stderr, "Falha de alocacao do cenario.\n");
        goto fim;
    }
    for (int i = 0; i < quantidade; i++) {
        if (!inserir_aresta_ponderada(g, arestas[i][0], arestas[i][1],
                                    arestas[i][2])) {
            fprintf(stderr, "Falha ao inserir aresta.\n");
            goto fim;
        }
    }
    printf("\n=== %s ===\n", nome);
    int testemunha;
    int retorno = bellman_ford_com_testemunha(g, 0, dist, pred, &testemunha);
    printf("Retorno: %d\n", retorno);
    if (retorno == -1) {
        goto fim;
    }
    if (retorno == 0) {
        printf("Ciclo negativo alcancavel; nao ha solucao de caminhos minimos.\n");
        printf("Ciclo: ");
        if (reconstruir_ciclo_negativo(n, pred, testemunha, stdout) == -1) {
            goto fim;
        }
        /* Neste cenario, o unico ciclo tem as arestas 1->2, 2->3, 3->1. */
        printf("Custo do ciclo: %d\n", arestas[1][2] + arestas[2][2] +
                                           arestas[3][2]);
    } else {
        for (int v = 0; v < n; v++) {
            if (dist[v] == INT_MAX) {
                printf("dist[%d] = infinito (inalcancavel), pred[%d] = %d\n",
                       v, v, pred[v]);
            } else {
                printf("dist[%d] = %d, pred[%d] = %d\n", v, dist[v], v, pred[v]);
            }
        }
    }
    resultado = EXIT_SUCCESS;
fim:
    liberar_grafo_ponderado(g);
    free(dist);
    free(pred);
    return resultado;
}

int main(void) {
    const int a[][3] = {{0, 1, 4}, {0, 2, 5}, {1, 3, 2},
                        {2, 1, -3}, {2, 3, 4}};
    const int b[][3] = {{0, 1, 1}, {1, 2, -2}, {2, 3, -2}, {3, 1, 1}};
    const int c[][3] = {{0, 1, 2}, {2, 3, -2}, {3, 2, 1}};
    int falhou = 0;
    falhou |= demonstrar("A: pesos negativos sem ciclo negativo", 5, a, 5);
    falhou |= demonstrar("B: ciclo negativo alcancavel", 4, b, 4);
    falhou |= demonstrar("C: ciclo negativo inalcancavel", 4, c, 3);
    return falhou ? EXIT_FAILURE : EXIT_SUCCESS;
}
