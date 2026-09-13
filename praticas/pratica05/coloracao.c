#include <stdlib.h>

#include "coloracao.h"

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

void inserir_aresta_lista(GrafoLista *g, int u, int v) {
    if (g == NULL || u < 0 || u >= g->n || v < 0 || v >= g->n ||
        u == v || sao_adjacentes(g, u, v)) {
        return;
    }

    No *primeiro = malloc(sizeof(*primeiro));
    No *segundo = malloc(sizeof(*segundo));
    if (primeiro == NULL || segundo == NULL) {
        free(primeiro);
        free(segundo);
        return;
    }
    primeiro->destino = v;
    primeiro->prox = g->adj[u];
    segundo->destino = u;
    segundo->prox = g->adj[v];
    g->adj[u] = primeiro;
    g->adj[v] = segundo;
}

int grau_vertice(GrafoLista *g, int vertice) {
    int grau = 0;
    for (No *no = g->adj[vertice]; no != NULL; no = no->prox) {
        grau++;
    }
    return grau;
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

static int *colorir_na_ordem(GrafoLista *g, int *ordem, int *num_cores) {
    int *cores = g->n > 0 ? malloc((size_t)g->n * sizeof(*cores)) : NULL;
    int *indisponivel = g->n > 0 ? calloc((size_t)g->n, sizeof(*indisponivel)) : NULL;
    if (g->n > 0 && (cores == NULL || indisponivel == NULL)) {
        free(cores);
        free(indisponivel);
        *num_cores = 0;
        return NULL;
    }
    for (int i = 0; i < g->n; i++) {
        cores[i] = -1;
    }

    *num_cores = 0;
    for (int i = 0; i < g->n; i++) {
        int u = ordem[i];
        for (No *no = g->adj[u]; no != NULL; no = no->prox) {
            int cor = cores[no->destino];
            if (cor >= 0) {
                indisponivel[cor] = 1;
            }
        }
        int cor = 0;
        while (indisponivel[cor]) {
            cor++;
        }
        cores[u] = cor;
        if (cor + 1 > *num_cores) {
            *num_cores = cor + 1;
        }
        for (No *no = g->adj[u]; no != NULL; no = no->prox) {
            int cor_vizinho = cores[no->destino];
            if (cor_vizinho >= 0) {
                indisponivel[cor_vizinho] = 0;
            }
        }
    }
    free(indisponivel);
    return cores;
}

int *coloracao_gulosa(GrafoLista *g, int *num_cores) {
    int *ordem = g->n > 0 ? malloc((size_t)g->n * sizeof(*ordem)) : NULL;
    if (g->n > 0 && ordem == NULL) {
        *num_cores = 0;
        return NULL;
    }
    for (int i = 0; i < g->n; i++) {
        ordem[i] = i;
    }
    int *cores = colorir_na_ordem(g, ordem, num_cores);
    free(ordem);
    return cores;
}

int *coloracao_welsh_powell(GrafoLista *g, int *num_cores) {
    int *graus = g->n > 0 ? malloc((size_t)g->n * sizeof(*graus)) : NULL;
    int *ordem = g->n > 0 ? malloc((size_t)g->n * sizeof(*ordem)) : NULL;
    if (g->n > 0 && (graus == NULL || ordem == NULL)) {
        free(graus);
        free(ordem);
        *num_cores = 0;
        return NULL;
    }
    for (int i = 0; i < g->n; i++) {
        graus[i] = grau_vertice(g, i);
        ordem[i] = i;
    }

    for (int i = 0; i < g->n - 1; i++) {
        int maior = i;
        for (int j = i + 1; j < g->n; j++) {
            if (graus[ordem[j]] > graus[ordem[maior]] ||
                (graus[ordem[j]] == graus[ordem[maior]] && ordem[j] < ordem[maior])) {
                maior = j;
            }
        }
        int temporario = ordem[i];
        ordem[i] = ordem[maior];
        ordem[maior] = temporario;
    }

    int *cores = colorir_na_ordem(g, ordem, num_cores);
    free(graus);
    free(ordem);
    return cores;
}

int eh_bipartido(GrafoLista *g) {
    int *cores = g->n > 0 ? malloc((size_t)g->n * sizeof(*cores)) : NULL;
    int *fila = g->n > 0 ? malloc((size_t)g->n * sizeof(*fila)) : NULL;
    if (g->n > 0 && (cores == NULL || fila == NULL)) {
        free(cores);
        free(fila);
        return 0;
    }
    for (int i = 0; i < g->n; i++) {
        cores[i] = -1;
    }

    for (int inicio = 0; inicio < g->n; inicio++) {
        if (cores[inicio] != -1) {
            continue;
        }
        int frente = 0;
        int fim = 0;
        cores[inicio] = 0;
        fila[fim++] = inicio;
        while (frente < fim) {
            int u = fila[frente++];
            for (No *no = g->adj[u]; no != NULL; no = no->prox) {
                int v = no->destino;
                if (cores[v] == -1) {
                    cores[v] = 1 - cores[u];
                    fila[fim++] = v;
                } else if (cores[v] == cores[u]) {
                    free(cores);
                    free(fila);
                    return 0;
                }
            }
        }
    }
    free(cores);
    free(fila);
    return 1;
}
