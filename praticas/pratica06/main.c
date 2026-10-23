#include <stdio.h>
#include <stdlib.h>

#include "dijkstra.h"

int main(void) {
    GrafoListaPonderado *g = criar_grafo_ponderado(5);
    int *dist = malloc(5 * sizeof(*dist));
    int *pred = malloc(5 * sizeof(*pred));
    int resultado = EXIT_FAILURE;
    if (g == NULL || dist == NULL || pred == NULL) {
        fprintf(stderr, "Falha de alocacao.\n");
        goto fim;
    }
    if (!inserir_aresta_ponderada(g, 0, 1, 4) ||
        !inserir_aresta_ponderada(g, 0, 2, 1) ||
        !inserir_aresta_ponderada(g, 1, 3, 1) ||
        !inserir_aresta_ponderada(g, 2, 1, 2) ||
        !inserir_aresta_ponderada(g, 2, 3, 5)) {
        fprintf(stderr, "Falha ao montar o digrafo.\n");
        goto fim;
    }
    dijkstra(g, 0, dist, pred);
    if (dist[0] != 0) {
        goto fim;
    }
    for (int v = 0; v < g->n; v++) {
        if (dist[v] == INT_MAX) {
            printf("dist[%d] = infinito (inalcancavel)\n", v);
        } else {
            printf("dist[%d] = %d\n", v, dist[v]);
        }
    }
    printf("Caminho de 0 ate 3: ");
    reconstruir_caminho(0, 3, pred);
    reconstruir_caminho(0, 4, pred);
    printf("Caminho de 0 ate 0: ");
    reconstruir_caminho(0, 0, pred);
    resultado = EXIT_SUCCESS;
fim:
    liberar_grafo_ponderado(g);
    free(dist);
    free(pred);
    return resultado;
}
