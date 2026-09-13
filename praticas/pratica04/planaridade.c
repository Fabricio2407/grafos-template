#include <stddef.h>

#include "planaridade.h"

static int adjacentes(GrafoLista *g, int u, int v) {
    for (No *no = g->adj[u]; no != NULL; no = no->prox) {
        if (no->destino == v) {
            return 1;
        }
    }
    return 0;
}

int eh_planar_euler(GrafoLista *g) {
    if (g->n < 3) {
        return 1;
    }
    return contar_arestas(g) <= 3 * g->n - 6;
}

static int forma_k5(GrafoLista *g, int a, int b, int c, int d, int e) {
    int vertices[5] = {a, b, c, d, e};
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (!adjacentes(g, vertices[i], vertices[j])) {
                return 0;
            }
        }
    }
    return 1;
}

static int grupos_disjuntos(int a, int b, int c, int d, int e, int f) {
    return a != d && a != e && a != f &&
           b != d && b != e && b != f &&
           c != d && c != e && c != f;
}

static int forma_k33(GrafoLista *g, int a, int b, int c,
                     int d, int e, int f) {
    int esquerda[3] = {a, b, c};
    int direita[3] = {d, e, f};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (!adjacentes(g, esquerda[i], direita[j])) {
                return 0;
            }
        }
    }
    return 1;
}

int possui_kuratowski_simples(GrafoLista *g) {
    if (g->n > 10) {
        return 0;
    }

    for (int a = 0; a < g->n; a++)
        for (int b = a + 1; b < g->n; b++)
            for (int c = b + 1; c < g->n; c++)
                for (int d = c + 1; d < g->n; d++)
                    for (int e = d + 1; e < g->n; e++)
                        if (forma_k5(g, a, b, c, d, e))
                            return 1;

    for (int a = 0; a < g->n; a++)
        for (int b = a + 1; b < g->n; b++)
            for (int c = b + 1; c < g->n; c++)
                for (int d = 0; d < g->n; d++)
                    for (int e = d + 1; e < g->n; e++)
                        for (int f = e + 1; f < g->n; f++)
                            if (grupos_disjuntos(a, b, c, d, e, f) &&
                                forma_k33(g, a, b, c, d, e, f))
                                return 1;
    return 0;
}
