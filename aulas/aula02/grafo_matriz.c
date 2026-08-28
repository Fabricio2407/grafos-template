#include <string.h>
#include <stdio.h>
#include "grafo_matriz.h"

void inicializar(GrafoMatriz *grafo, int numero) {
    memset(grafo, 0, sizeof(GrafoMatriz));
    grafo->num_vertices = numero;
}

void inserir_aresta(GrafoMatriz *grafo, int u, int v) {
    grafo->adjacente[u][v] = 1;
    grafo->adjacente[v][u] = 1;
}

void inserir_arco(GrafoMatriz *grafo, int u, int v) {
    grafo->adjacente[u][v] = 1;
}

void exibir_matriz(GrafoMatriz *grafo) {
    for(int i = 0; i < grafo->num_vertices; i++) {
        for(int j = 0; j < grafo->num_vertices; j++) {
            printf("%3i", grafo->adjacente[i][j]);
        }
        printf("\n");
    }
}