#include <stdio.h>
#include <stdlib.h>

#include "busca_largura.h"
#include "busca_profundidade.h"

static void demonstrar_grafo(const char *nome, GrafoLista *g) {
    int *dist = malloc((size_t)g->n * sizeof(*dist));
    int *pred = malloc((size_t)g->n * sizeof(*pred));
    int *visitado = calloc((size_t)g->n, sizeof(*visitado));
    int *entrada = calloc((size_t)g->n, sizeof(*entrada));
    int *saida = calloc((size_t)g->n, sizeof(*saida));
    if (dist == NULL || pred == NULL || visitado == NULL || entrada == NULL || saida == NULL) {
        free(dist);
        free(pred);
        free(visitado);
        free(entrada);
        free(saida);
        return;
    }

    printf("\n=== %s ===\n", nome);
    printf("\nBFS a partir do vertice 0\n");
    bfs(g, 0, dist, pred);
    for (int i = 0; i < g->n; i++) {
        printf("Vertice %d: distancia = %d, predecessor = %d\n", i, dist[i], pred[i]);
    }

    int tempo = 0;
    for (int i = 0; i < g->n; i++) {
        if (!visitado[i]) {
            dfs_recursiva(g, i, visitado, entrada, saida, &tempo);
        }
    }
    printf("\nDFS recursiva\n");
    for (int i = 0; i < g->n; i++) {
        printf("Vertice %d: entrada = %d, saida = %d\n", i, entrada[i], saida[i]);
    }

    printf("\nComponentes conexos: %d\n", contar_componentes(g));
    printf("Possui ciclo: %s\n", tem_ciclo(g) ? "sim" : "nao");
    printf("Eh bipartido: %s\n", eh_bipartido(g) ? "sim" : "nao");

    free(dist);
    free(pred);
    free(visitado);
    free(entrada);
    free(saida);
}

int main(void) {
    GrafoLista *caminho = criar_grafo_lista(5);
    GrafoLista *triangulo = criar_grafo_lista(3);
    GrafoLista *desconectado = criar_grafo_lista(6);
    if (caminho == NULL || triangulo == NULL || desconectado == NULL) {
        liberar_grafo_lista(caminho);
        liberar_grafo_lista(triangulo);
        liberar_grafo_lista(desconectado);
        return 1;
    }

    inserir_aresta_lista(caminho, 0, 1);
    inserir_aresta_lista(caminho, 1, 2);
    inserir_aresta_lista(caminho, 2, 3);
    inserir_aresta_lista(caminho, 3, 4);

    inserir_aresta_lista(triangulo, 0, 1);
    inserir_aresta_lista(triangulo, 1, 2);
    inserir_aresta_lista(triangulo, 2, 0);

    inserir_aresta_lista(desconectado, 0, 1);
    inserir_aresta_lista(desconectado, 1, 2);
    inserir_aresta_lista(desconectado, 3, 4);

    demonstrar_grafo("Grafo 1: caminho conexo", caminho);
    demonstrar_grafo("Grafo 2: triangulo", triangulo);
    demonstrar_grafo("Grafo 3: componentes desconectados", desconectado);

    liberar_grafo_lista(caminho);
    liberar_grafo_lista(triangulo);
    liberar_grafo_lista(desconectado);
    return 0;
}
