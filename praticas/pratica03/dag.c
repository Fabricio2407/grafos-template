#include <stdlib.h>

#include "dag.h"

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

void inserir_aresta_direcionada(GrafoLista *g, int origem, int destino) {
    if (g == NULL || origem < 0 || origem >= g->n || destino < 0 || destino >= g->n) {
        return;
    }

    No *novo = malloc(sizeof(*novo));
    if (novo == NULL) {
        return;
    }
    novo->destino = destino;
    novo->prox = g->adj[origem];
    g->adj[origem] = novo;
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

int *ordenacao_topologica_kahn(GrafoLista *g, int *tamanho) {
    *tamanho = 0;
    int *grau_entrada = calloc((size_t)g->n, sizeof(*grau_entrada));
    int *fila = g->n > 0 ? malloc((size_t)g->n * sizeof(*fila)) : NULL;
    int *ordem = g->n > 0 ? malloc((size_t)g->n * sizeof(*ordem)) : NULL;
    if (g->n > 0 && (grau_entrada == NULL || fila == NULL || ordem == NULL)) {
        free(grau_entrada);
        free(fila);
        free(ordem);
        return NULL;
    }

    for (int u = 0; u < g->n; u++) {
        for (No *no = g->adj[u]; no != NULL; no = no->prox) {
            grau_entrada[no->destino]++;
        }
    }

    int inicio = 0;
    int fim = 0;
    for (int i = 0; i < g->n; i++) {
        if (grau_entrada[i] == 0) {
            fila[fim++] = i;
        }
    }

    while (inicio < fim) {
        int u = fila[inicio++];
        ordem[*tamanho] = u;
        (*tamanho)++;
        for (No *no = g->adj[u]; no != NULL; no = no->prox) {
            int v = no->destino;
            grau_entrada[v]--;
            if (grau_entrada[v] == 0) {
                fila[fim++] = v;
            }
        }
    }

    free(grau_entrada);
    free(fila);
    if (*tamanho != g->n) {
        free(ordem);
        *tamanho = 0;
        return NULL;
    }
    return ordem;
}

static int visitar_dfs(GrafoLista *g, int u, int *estado,
                       int *ordem, int *posicao) {
    estado[u] = 1;
    for (No *no = g->adj[u]; no != NULL; no = no->prox) {
        int v = no->destino;
        if (estado[v] == 1) {
            return 0;
        }
        if (estado[v] == 0 && !visitar_dfs(g, v, estado, ordem, posicao)) {
            return 0;
        }
    }
    estado[u] = 2;
    ordem[(*posicao)--] = u;
    return 1;
}

int *ordenacao_topologica_dfs(GrafoLista *g, int *tamanho) {
    *tamanho = 0;
    int *estado = calloc((size_t)g->n, sizeof(*estado));
    int *ordem = g->n > 0 ? malloc((size_t)g->n * sizeof(*ordem)) : NULL;
    if (g->n > 0 && (estado == NULL || ordem == NULL)) {
        free(estado);
        free(ordem);
        return NULL;
    }

    int posicao = g->n - 1;
    for (int i = 0; i < g->n; i++) {
        if (estado[i] == 0 && !visitar_dfs(g, i, estado, ordem, &posicao)) {
            free(estado);
            free(ordem);
            return NULL;
        }
    }

    free(estado);
    *tamanho = g->n;
    return ordem;
}

int eh_dag(GrafoLista *g) {
    int tamanho;
    int *ordem = ordenacao_topologica_dfs(g, &tamanho);
    int resultado = ordem != NULL || g->n == 0;
    free(ordem);
    return resultado;
}
