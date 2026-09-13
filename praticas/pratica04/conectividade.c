#include <stdlib.h>

#include "conectividade.h"

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

static int sao_adjacentes(GrafoLista *g, int u, int v) {
    for (No *no = g->adj[u]; no != NULL; no = no->prox) {
        if (no->destino == v) {
            return 1;
        }
    }
    return 0;
}

static int adicionar_no(GrafoLista *g, int u, int v) {
    No *novo = malloc(sizeof(*novo));
    if (novo == NULL) {
        return 0;
    }
    novo->destino = v;
    novo->prox = g->adj[u];
    g->adj[u] = novo;
    return 1;
}

void inserir_aresta_lista(GrafoLista *g, int u, int v) {
    if (g == NULL || u < 0 || u >= g->n || v < 0 || v >= g->n ||
        u == v || sao_adjacentes(g, u, v)) {
        return;
    }
    if (adicionar_no(g, u, v)) {
        adicionar_no(g, v, u);
    }
}

int contar_arestas(GrafoLista *g) {
    int total = 0;
    for (int u = 0; u < g->n; u++) {
        for (No *no = g->adj[u]; no != NULL; no = no->prox) {
            total++;
        }
    }
    return total / 2;
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

static int minimo(int a, int b) {
    return a < b ? a : b;
}

void dfs_articulacoes(GrafoLista *g, int u, int *visitado,
                      int *descoberta, int *low, int *pai,
                      int *articulacao, int *tempo) {
    int filhos = 0;
    visitado[u] = 1;
    descoberta[u] = low[u] = ++(*tempo);

    for (No *no = g->adj[u]; no != NULL; no = no->prox) {
        int v = no->destino;
        if (!visitado[v]) {
            filhos++;
            pai[v] = u;
            dfs_articulacoes(g, v, visitado, descoberta, low, pai,
                             articulacao, tempo);
            low[u] = minimo(low[u], low[v]);
            if (pai[u] == -1 && filhos > 1) {
                articulacao[u] = 1;
            }
            if (pai[u] != -1 && low[v] >= descoberta[u]) {
                articulacao[u] = 1;
            }
        } else if (v != pai[u]) {
            low[u] = minimo(low[u], descoberta[v]);
        }
    }
}

int *encontrar_articulacoes(GrafoLista *g) {
    int *visitado = calloc((size_t)g->n, sizeof(*visitado));
    int *descoberta = calloc((size_t)g->n, sizeof(*descoberta));
    int *low = calloc((size_t)g->n, sizeof(*low));
    int *pai = malloc((size_t)g->n * sizeof(*pai));
    int *articulacao = calloc((size_t)g->n, sizeof(*articulacao));
    if (g->n > 0 && (visitado == NULL || descoberta == NULL || low == NULL ||
                     pai == NULL || articulacao == NULL)) {
        free(visitado); free(descoberta); free(low); free(pai); free(articulacao);
        return NULL;
    }
    for (int i = 0; i < g->n; i++) {
        pai[i] = -1;
    }
    int tempo = 0;
    for (int i = 0; i < g->n; i++) {
        if (!visitado[i]) {
            dfs_articulacoes(g, i, visitado, descoberta, low, pai,
                             articulacao, &tempo);
        }
    }
    free(visitado); free(descoberta); free(low); free(pai);
    return articulacao;
}

static void dfs_pontes(GrafoLista *g, int u, int pai, int *visitado,
                       int *descoberta, int *low, int *tempo,
                       Ponte *pontes, int *quantidade) {
    visitado[u] = 1;
    descoberta[u] = low[u] = ++(*tempo);
    for (No *no = g->adj[u]; no != NULL; no = no->prox) {
        int v = no->destino;
        if (!visitado[v]) {
            dfs_pontes(g, v, u, visitado, descoberta, low, tempo,
                       pontes, quantidade);
            low[u] = minimo(low[u], low[v]);
            if (low[v] > descoberta[u]) {
                pontes[*quantidade].u = u;
                pontes[*quantidade].v = v;
                (*quantidade)++;
            }
        } else if (v != pai) {
            low[u] = minimo(low[u], descoberta[v]);
        }
    }
}

Ponte *detectar_pontes(GrafoLista *g, int *quantidade) {
    *quantidade = 0;
    int *visitado = calloc((size_t)g->n, sizeof(*visitado));
    int *descoberta = calloc((size_t)g->n, sizeof(*descoberta));
    int *low = calloc((size_t)g->n, sizeof(*low));
    int maximo = contar_arestas(g);
    Ponte *pontes = maximo > 0 ? malloc((size_t)maximo * sizeof(*pontes)) : NULL;
    if (g->n > 0 && (visitado == NULL || descoberta == NULL || low == NULL)) {
        free(visitado); free(descoberta); free(low); free(pontes);
        return NULL;
    }
    if (maximo > 0 && pontes == NULL) {
        free(visitado); free(descoberta); free(low);
        return NULL;
    }
    int tempo = 0;
    for (int i = 0; i < g->n; i++) {
        if (!visitado[i]) {
            dfs_pontes(g, i, -1, visitado, descoberta, low, &tempo,
                       pontes, quantidade);
        }
    }
    free(visitado); free(descoberta); free(low);
    return pontes;
}
