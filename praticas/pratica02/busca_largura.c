#include <stdlib.h>

#include "busca_largura.h"

static Fila *criar_fila(int capacidade) {
    Fila *fila = malloc(sizeof(*fila));
    if (fila == NULL) {
        return NULL;
    }
    fila->dados = capacidade > 0 ? malloc((size_t)capacidade * sizeof(*fila->dados)) : NULL;
    if (capacidade > 0 && fila->dados == NULL) {
        free(fila);
        return NULL;
    }
    fila->capacidade = capacidade;
    fila->inicio = 0;
    fila->fim = 0;
    fila->tamanho = 0;
    return fila;
}

static void liberar_fila(Fila *fila) {
    if (fila != NULL) {
        free(fila->dados);
        free(fila);
    }
}

static void enfileirar(Fila *fila, int valor) {
    fila->dados[fila->fim] = valor;
    fila->fim = (fila->fim + 1) % fila->capacidade;
    fila->tamanho++;
}

static int desenfileirar(Fila *fila) {
    int valor = fila->dados[fila->inicio];
    fila->inicio = (fila->inicio + 1) % fila->capacidade;
    fila->tamanho--;
    return valor;
}

GrafoLista *criar_grafo_lista(int n) {
    if (n < 0) {
        return NULL;
    }
    GrafoLista *g = malloc(sizeof(*g));
    if (g == NULL) {
        return NULL;
    }
    g->n = n;
    g->adj = n > 0 ? calloc((size_t)n, sizeof(*g->adj)) : NULL;
    if (n > 0 && g->adj == NULL) {
        free(g);
        return NULL;
    }
    return g;
}

static int adicionar_no(GrafoLista *g, int origem, int destino) {
    No *novo = malloc(sizeof(*novo));
    if (novo == NULL) {
        return 0;
    }
    novo->destino = destino;
    novo->prox = g->adj[origem];
    g->adj[origem] = novo;
    return 1;
}

void inserir_aresta_lista(GrafoLista *g, int origem, int destino) {
    if (g == NULL || origem < 0 || origem >= g->n || destino < 0 || destino >= g->n) {
        return;
    }
    if (adicionar_no(g, origem, destino) && origem != destino) {
        adicionar_no(g, destino, origem);
    }
}

void liberar_grafo_lista(GrafoLista *g) {
    if (g == NULL) {
        return;
    }
    for (int i = 0; i < g->n; i++) {
        No *no = g->adj[i];
        while (no != NULL) {
            No *proximo = no->prox;
            free(no);
            no = proximo;
        }
    }
    free(g->adj);
    free(g);
}

void bfs(GrafoLista *g, int origem, int *dist, int *pred) {
    for (int i = 0; i < g->n; i++) {
        dist[i] = -1;
        pred[i] = -1;
    }
    if (origem < 0 || origem >= g->n) {
        return;
    }

    Fila *fila = criar_fila(g->n);
    if (fila == NULL) {
        return;
    }
    dist[origem] = 0;
    enfileirar(fila, origem);

    while (fila->tamanho > 0) {
        int u = desenfileirar(fila);
        for (No *no = g->adj[u]; no != NULL; no = no->prox) {
            int v = no->destino;
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                pred[v] = u;
                enfileirar(fila, v);
            }
        }
    }
    liberar_fila(fila);
}

int eh_bipartido(GrafoLista *g) {
    int *cor = malloc((size_t)g->n * sizeof(*cor));
    Fila *fila = criar_fila(g->n);
    if ((g->n > 0 && cor == NULL) || fila == NULL) {
        free(cor);
        liberar_fila(fila);
        return 0;
    }
    for (int i = 0; i < g->n; i++) {
        cor[i] = -1;
    }

    for (int inicio = 0; inicio < g->n; inicio++) {
        if (cor[inicio] != -1) {
            continue;
        }
        cor[inicio] = 0;
        enfileirar(fila, inicio);
        while (fila->tamanho > 0) {
            int u = desenfileirar(fila);
            for (No *no = g->adj[u]; no != NULL; no = no->prox) {
                int v = no->destino;
                if (cor[v] == -1) {
                    cor[v] = 1 - cor[u];
                    enfileirar(fila, v);
                } else if (cor[v] == cor[u]) {
                    free(cor);
                    liberar_fila(fila);
                    return 0;
                }
            }
        }
    }
    free(cor);
    liberar_fila(fila);
    return 1;
}
