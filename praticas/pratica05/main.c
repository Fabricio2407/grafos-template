#include <stdio.h>
#include <stdlib.h>

#include "coloracao.h"

static int coloracao_valida(GrafoLista *g, int *cores) {
    if (cores == NULL) {
        return 0;
    }
    for (int u = 0; u < g->n; u++) {
        for (No *no = g->adj[u]; no != NULL; no = no->prox) {
            if (cores[u] == cores[no->destino]) {
                return 0;
            }
        }
    }
    return 1;
}

static void mostrar_cores(const char *nome, GrafoLista *g,
                          int *cores, int num_cores) {
    printf("%s:\n", nome);
    for (int i = 0; i < g->n; i++) {
        printf("Vertice %d -> cor %d\n", i, cores[i]);
    }
    printf("Numero de cores: %d\n", num_cores);
    printf("Coloracao valida: %s\n", coloracao_valida(g, cores) ? "sim" : "nao");
}

static void demonstrar_grafo(const char *nome, GrafoLista *g) {
    int cores_gulosa;
    int cores_welsh;
    int *gulosa = coloracao_gulosa(g, &cores_gulosa);
    int *welsh = coloracao_welsh_powell(g, &cores_welsh);

    printf("\n=== %s ===\n\n", nome);
    mostrar_cores("Coloracao gulosa", g, gulosa, cores_gulosa);
    printf("\n");
    mostrar_cores("Welsh-Powell", g, welsh, cores_welsh);
    printf("\nEh bipartido: %s\n", eh_bipartido(g) ? "sim" : "nao");

    free(gulosa);
    free(welsh);
}

int main(void) {
    GrafoLista *ciclo_par = criar_grafo_lista(4);
    GrafoLista *triangulo = criar_grafo_lista(3);
    GrafoLista *comparacao = criar_grafo_lista(6);
    if (ciclo_par == NULL || triangulo == NULL || comparacao == NULL) {
        liberar_grafo_lista(ciclo_par);
        liberar_grafo_lista(triangulo);
        liberar_grafo_lista(comparacao);
        return 1;
    }

    inserir_aresta_lista(ciclo_par, 0, 1);
    inserir_aresta_lista(ciclo_par, 1, 2);
    inserir_aresta_lista(ciclo_par, 2, 3);
    inserir_aresta_lista(ciclo_par, 3, 0);

    inserir_aresta_lista(triangulo, 0, 1);
    inserir_aresta_lista(triangulo, 1, 2);
    inserir_aresta_lista(triangulo, 2, 0);

    inserir_aresta_lista(comparacao, 0, 1);
    inserir_aresta_lista(comparacao, 0, 2);
    inserir_aresta_lista(comparacao, 0, 3);
    inserir_aresta_lista(comparacao, 1, 4);
    inserir_aresta_lista(comparacao, 2, 4);
    inserir_aresta_lista(comparacao, 3, 5);
    inserir_aresta_lista(comparacao, 4, 5);

    demonstrar_grafo("EXEMPLO 1: CICLO PAR", ciclo_par);
    demonstrar_grafo("EXEMPLO 2: TRIANGULO", triangulo);
    demonstrar_grafo("EXEMPLO 3: COMPARACAO", comparacao);

    liberar_grafo_lista(ciclo_par);
    liberar_grafo_lista(triangulo);
    liberar_grafo_lista(comparacao);
    return 0;
}
