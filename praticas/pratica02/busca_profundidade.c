#include <stdlib.h>

#include "busca_profundidade.h"

void dfs_recursiva(GrafoLista *g, int u, int *visitado,
                   int *entrada, int *saida, int *tempo) {
    visitado[u] = 1;
    entrada[u] = ++(*tempo);
    for (No *no = g->adj[u]; no != NULL; no = no->prox) {
        if (!visitado[no->destino]) {
            dfs_recursiva(g, no->destino, visitado, entrada, saida, tempo);
        }
    }
    saida[u] = ++(*tempo);
}

static void visitar_componente(GrafoLista *g, int u, int *visitado) {
    visitado[u] = 1;
    for (No *no = g->adj[u]; no != NULL; no = no->prox) {
        if (!visitado[no->destino]) {
            visitar_componente(g, no->destino, visitado);
        }
    }
}

int contar_componentes(GrafoLista *g) {
    int *visitado = calloc((size_t)g->n, sizeof(*visitado));
    if (g->n > 0 && visitado == NULL) {
        return 0;
    }
    int componentes = 0;
    for (int i = 0; i < g->n; i++) {
        if (!visitado[i]) {
            componentes++;
            visitar_componente(g, i, visitado);
        }
    }
    free(visitado);
    return componentes;
}

static int ciclo_dfs(GrafoLista *g, int u, int pai, int *visitado) {
    visitado[u] = 1;
    for (No *no = g->adj[u]; no != NULL; no = no->prox) {
        int v = no->destino;
        if (!visitado[v]) {
            if (ciclo_dfs(g, v, u, visitado)) {
                return 1;
            }
        } else if (v != pai) {
            return 1;
        }
    }
    return 0;
}

int tem_ciclo(GrafoLista *g) {
    int *visitado = calloc((size_t)g->n, sizeof(*visitado));
    if (g->n > 0 && visitado == NULL) {
        return 0;
    }
    for (int i = 0; i < g->n; i++) {
        if (!visitado[i] && ciclo_dfs(g, i, -1, visitado)) {
            free(visitado);
            return 1;
        }
    }
    free(visitado);
    return 0;
}
