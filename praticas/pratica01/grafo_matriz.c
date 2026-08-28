#include <stdio.h>
#include <stdlib.h>

#include "grafo_matriz.h"

static int vertice_valido(const GrafoMatriz *grafo, int vertice) {
    return grafo != NULL && vertice >= 0 && vertice < grafo->n;
}

GrafoMatriz *criar_grafo_matriz(int n) {
    if (n < 0) {
        return NULL;
    }

    GrafoMatriz *grafo = malloc(sizeof(*grafo));
    if (grafo == NULL) {
        return NULL;
    }

    grafo->n = n;
    grafo->adj = NULL;
    if (n == 0) {
        return grafo;
    }

    grafo->adj = calloc((size_t)n, sizeof(*grafo->adj));
    if (grafo->adj == NULL) {
        free(grafo);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        grafo->adj[i] = calloc((size_t)n, sizeof(*grafo->adj[i]));
        if (grafo->adj[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(grafo->adj[j]);
            }
            free(grafo->adj);
            free(grafo);
            return NULL;
        }
    }

    return grafo;
}

void inserir_aresta_matriz(GrafoMatriz *grafo, int origem, int destino) {
    if (!vertice_valido(grafo, origem) || !vertice_valido(grafo, destino)) {
        return;
    }

    grafo->adj[origem][destino] = 1;
    grafo->adj[destino][origem] = 1;
}

void remover_aresta_matriz(GrafoMatriz *grafo, int origem, int destino) {
    if (!vertice_valido(grafo, origem) || !vertice_valido(grafo, destino)) {
        return;
    }

    grafo->adj[origem][destino] = 0;
    grafo->adj[destino][origem] = 0;
}

int grau_matriz(const GrafoMatriz *grafo, int vertice) {
    if (!vertice_valido(grafo, vertice)) {
        return -1;
    }

    int grau = 0;
    for (int destino = 0; destino < grafo->n; destino++) {
        grau += grafo->adj[vertice][destino];
    }
    return grau;
}

int sao_adjacentes_matriz(const GrafoMatriz *grafo, int origem, int destino) {
    if (!vertice_valido(grafo, origem) || !vertice_valido(grafo, destino)) {
        return 0;
    }
    return grafo->adj[origem][destino] != 0;
}

void exibir_matriz(const GrafoMatriz *grafo) {
    if (grafo == NULL) {
        return;
    }

    for (int origem = 0; origem < grafo->n; origem++) {
        for (int destino = 0; destino < grafo->n; destino++) {
            printf("%3d", grafo->adj[origem][destino]);
        }
        printf("\n");
    }
}

void liberar_grafo_matriz(GrafoMatriz *grafo) {
    if (grafo == NULL) {
        return;
    }

    for (int i = 0; i < grafo->n; i++) {
        free(grafo->adj[i]);
    }
    free(grafo->adj);
    free(grafo);
}
