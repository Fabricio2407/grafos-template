#include <stdio.h>
#include <stdlib.h>

#include "conectividade.h"
#include "planaridade.h"

static void mostrar_conectividade(GrafoLista *g) {
    int *articulacoes = encontrar_articulacoes(g);
    int quantidade = 0;
    Ponte *pontes = detectar_pontes(g, &quantidade);

    printf("Articulacoes:\n");
    for (int i = 0; i < g->n; i++) {
        if (articulacoes != NULL && articulacoes[i]) {
            printf("%d\n", i);
        }
    }
    printf("\nPontes:\n");
    for (int i = 0; i < quantidade; i++) {
        printf("%d - %d\n", pontes[i].u, pontes[i].v);
    }

    free(articulacoes);
    free(pontes);
}

static void mostrar_planaridade(GrafoLista *g) {
    int euler = eh_planar_euler(g);
    int kuratowski = possui_kuratowski_simples(g);
    printf("Euler: %s\n", euler ? "atende ao limite" : "viola o limite");
    printf("Kuratowski simples: %s\n", kuratowski ? "encontrado" : "nao encontrado");
    printf("Resultado da heuristica: %s\n",
           (!euler || kuratowski) ? "nao planar" : "possivelmente planar");
}

int main(void) {
    GrafoLista *conectividade = criar_grafo_lista(5);
    GrafoLista *planar = criar_grafo_lista(4);
    GrafoLista *k33 = criar_grafo_lista(6);
    if (conectividade == NULL || planar == NULL || k33 == NULL) {
        liberar_grafo_lista(conectividade);
        liberar_grafo_lista(planar);
        liberar_grafo_lista(k33);
        return 1;
    }

    inserir_aresta_lista(conectividade, 0, 1);
    inserir_aresta_lista(conectividade, 1, 2);
    inserir_aresta_lista(conectividade, 1, 3);
    inserir_aresta_lista(conectividade, 3, 4);

    inserir_aresta_lista(planar, 0, 1);
    inserir_aresta_lista(planar, 1, 2);
    inserir_aresta_lista(planar, 2, 3);
    inserir_aresta_lista(planar, 3, 0);

    for (int u = 0; u < 3; u++) {
        for (int v = 3; v < 6; v++) {
            inserir_aresta_lista(k33, u, v);
        }
    }

    printf("=== EXEMPLO 1: CONECTIVIDADE ===\n\n");
    mostrar_conectividade(conectividade);
    printf("\n=== EXEMPLO 2: GRAFO PLANAR ===\n\n");
    mostrar_planaridade(planar);
    printf("\n=== EXEMPLO 3: K3,3 ===\n\n");
    mostrar_planaridade(k33);

    liberar_grafo_lista(conectividade);
    liberar_grafo_lista(planar);
    liberar_grafo_lista(k33);
    return 0;
}
