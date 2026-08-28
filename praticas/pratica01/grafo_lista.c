#include <stdio.h>
#include <stdlib.h>

#include "grafo_lista.h"

static int vertice_valido(const GrafoLista *grafo, int vertice) {
    return grafo != NULL && vertice >= 0 && vertice < grafo->n;
}

static No *buscar_no(const GrafoLista *grafo, int origem, int destino) {
    for (No *no = grafo->adj[origem]; no != NULL; no = no->prox) {
        if (no->destino == destino) {
            return no;
        }
    }
    return NULL;
}

static int adicionar_no(GrafoLista *grafo, int origem, int destino) {
    if (buscar_no(grafo, origem, destino) != NULL) {
        return 1;
    }

    No *novo = malloc(sizeof(*novo));
    if (novo == NULL) {
        return 0;
    }
    novo->destino = destino;
    novo->prox = grafo->adj[origem];
    grafo->adj[origem] = novo;
    return 1;
}

static void remover_no(GrafoLista *grafo, int origem, int destino) {
    No **atual = &grafo->adj[origem];
    while (*atual != NULL) {
        if ((*atual)->destino == destino) {
            No *removido = *atual;
            *atual = removido->prox;
            free(removido);
            return;
        }
        atual = &(*atual)->prox;
    }
}

GrafoLista *criar_grafo_lista(int n) {
    if (n < 0) {
        return NULL;
    }

    GrafoLista *grafo = malloc(sizeof(*grafo));
    if (grafo == NULL) {
        return NULL;
    }

    grafo->n = n;
    grafo->adj = n == 0 ? NULL : calloc((size_t)n, sizeof(*grafo->adj));
    if (n > 0 && grafo->adj == NULL) {
        free(grafo);
        return NULL;
    }
    return grafo;
}

void inserir_aresta_lista(GrafoLista *grafo, int origem, int destino) {
    if (!vertice_valido(grafo, origem) || !vertice_valido(grafo, destino)) {
        return;
    }

    if (!adicionar_no(grafo, origem, destino)) {
        return;
    }
    if (origem != destino) {
        adicionar_no(grafo, destino, origem);
    }
}

void remover_aresta_lista(GrafoLista *grafo, int origem, int destino) {
    if (!vertice_valido(grafo, origem) || !vertice_valido(grafo, destino)) {
        return;
    }

    remover_no(grafo, origem, destino);
    if (origem != destino) {
        remover_no(grafo, destino, origem);
    }
}

int grau_lista(const GrafoLista *grafo, int vertice) {
    if (!vertice_valido(grafo, vertice)) {
        return -1;
    }

    int grau = 0;
    for (No *no = grafo->adj[vertice]; no != NULL; no = no->prox) {
        grau++;
    }
    return grau;
}

int sao_adjacentes_lista(const GrafoLista *grafo, int origem, int destino) {
    if (!vertice_valido(grafo, origem) || !vertice_valido(grafo, destino)) {
        return 0;
    }
    return buscar_no(grafo, origem, destino) != NULL;
}

void exibir_lista(const GrafoLista *grafo) {
    if (grafo == NULL) {
        return;
    }

    for (int origem = 0; origem < grafo->n; origem++) {
        printf("%d:", origem);
        for (No *no = grafo->adj[origem]; no != NULL; no = no->prox) {
            printf(" %d", no->destino);
        }
        printf("\n");
    }
}

void liberar_grafo_lista(GrafoLista *grafo) {
    if (grafo == NULL) {
        return;
    }

    for (int i = 0; i < grafo->n; i++) {
        No *no = grafo->adj[i];
        while (no != NULL) {
            No *proximo = no->prox;
            free(no);
            no = proximo;
        }
    }
    free(grafo->adj);
    free(grafo);
}
