#include <stdlib.h>
#include "grafo_lista.h"


GrafoLista *criar_grafo(int n) {
    GrafoLista *g = (GrafoLista *)malloc(sizeof(GrafoLista));
    g->num_vertices = n;
    g->lista = (No **)malloc(sizeof(No)*n);
    for(int i = 0; i < n; i++) {
        g->lista[i] = NULL;
    }
    return g;
}

void adicionar_aresta(GrafoLista *g, int u, int v) {
    No *proximo = g->lista[u];
    No *novo = (No *)malloc(sizeof(No));
    novo->vertice = v;
    novo->proximo = proximo;
    g->lista[u] = novo;

    proximo = g->lista[v];
    novo = (No *)malloc(sizeof(No));
    novo->vertice = u;
    novo->proximo = proximo;
    g->lista[v] = novo;
}