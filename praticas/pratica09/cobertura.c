#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cobertura.h"

struct Grafo {
    int n, m;
    Aresta *arestas;
};

static void *alocar(int n, size_t tamanho) {
    if (n <= 0 || (size_t)n > SIZE_MAX / tamanho) {
        return NULL;
    }
    return calloc((size_t)n, tamanho);
}

Grafo *criar_grafo(int n, const Aresta *arestas, int m) {
    if (n < 0 || m < 0 || (m > 0 && arestas == NULL)) {
        return NULL;
    }
    for (int i = 0; i < m; i++) {
        int u = arestas[i].u, v = arestas[i].v;
        if (u < 0 || u >= n || v < 0 || v >= n || u == v) {
            return NULL;
        }
        for (int j = 0; j < i; j++) {
            Aresta b = arestas[j];
            if ((u == b.u && v == b.v) || (u == b.v && v == b.u)) {
                return NULL;
            }
        }
    }
    Grafo *g = calloc(1, sizeof(*g));
    if (g == NULL) {
        return NULL;
    }
    g->n = n;
    g->m = m;
    if (m > 0) {
        g->arestas = alocar(m, sizeof(*g->arestas));
        if (g->arestas == NULL) {
            liberar_grafo(g);
            return NULL;
        }
        memcpy(g->arestas, arestas, (size_t)m * sizeof(*arestas));
    }
    return g;
}

void liberar_grafo(Grafo *g) {
    if (g != NULL) {
        free(g->arestas);
        free(g);
    }
}

void liberar_resultado_cobertura(ResultadoCobertura *r) {
    if (r != NULL) {
        free(r->vertices);
        *r = (ResultadoCobertura){.status = COBERTURA_ENTRADA_INVALIDA};
    }
}

int eh_cobertura(const Grafo *g, const int *vertices, int quantidade) {
    if (g == NULL || quantidade < 0 || quantidade > g->n ||
        (quantidade > 0 && vertices == NULL)) {
        return -1;
    }
    for (int i = 0; i < quantidade; i++) {
        if (vertices[i] < 0 || vertices[i] >= g->n) {
            return -1;
        }
        for (int j = 0; j < i; j++) {
            if (vertices[j] == vertices[i]) {
                return -1;
            }
        }
    }
    int completa = 1;
    for (int i = 0; i < g->m; i++) {
        Aresta a = g->arestas[i];
        int coberta = 0;
        for (int j = 0; j < quantidade; j++) {
            if (vertices[j] == a.u || vertices[j] == a.v) {
                coberta = 1;
                break;
            }
        }
        if (!coberta) {
            completa = 0;
        }
    }
    return completa;
}

ResultadoCobertura cobertura_gulosa(const Grafo *g) {
    ResultadoCobertura r = {.status = COBERTURA_ENTRADA_INVALIDA};
    if (g == NULL) {
        return r;
    }
    r.status = COBERTURA_SUCESSO;
    if (g->m == 0) {
        return r;
    }
    r.vertices = alocar(g->n, sizeof(*r.vertices));
    unsigned char *selecionados = alocar(g->n, sizeof(*selecionados));
    int *graus = alocar(g->n, sizeof(*graus));
    if (r.vertices == NULL || selecionados == NULL || graus == NULL) {
        free(selecionados);
        free(graus);
        liberar_resultado_cobertura(&r);
        r.status = COBERTURA_SEM_MEMORIA;
        return r;
    }
    for (;;) {
        memset(graus, 0, (size_t)g->n * sizeof(*graus));
        /* Recalcular equivale a retirar todas as arestas ja cobertas;
         * cada aresta descoberta contribui uma vez para cada extremo. */
        for (int i = 0; i < g->m; i++) {
            Aresta a = g->arestas[i];
            if (!selecionados[a.u] && !selecionados[a.v]) {
                graus[a.u]++;
                graus[a.v]++;
            }
        }
        int melhor = -1;
        for (int v = 0; v < g->n; v++) {
            if (graus[v] > 0 && (melhor == -1 || graus[v] > graus[melhor])) {
                melhor = v;
            }
        }
        if (melhor == -1) {
            break;
        }
        selecionados[melhor] = 1;
        r.vertices[r.quantidade++] = melhor;
    }
    free(selecionados);
    free(graus);
    return r;
}

typedef struct {
    const Grafo *g;
    unsigned char *selecionados;
    int *parcial;
    ResultadoCobertura *melhor;
} Busca;

static void buscar(Busca *b, int quantidade) {
    if (quantidade >= b->melhor->quantidade) {
        return;
    }
    int i;
    for (i = 0; i < b->g->m; i++) {
        Aresta a = b->g->arestas[i];
        if (!b->selecionados[a.u] && !b->selecionados[a.v]) {
            break;
        }
    }
    if (i == b->g->m) {
        memcpy(b->melhor->vertices, b->parcial,
               (size_t)quantidade * sizeof(*b->parcial));
        b->melhor->quantidade = quantidade;
        return;
    }
    /* Toda cobertura inclui u ou v. Ambos estao ausentes do parcial;
     * desfazer cada inclusao restaura integralmente o estado do pai. */
    Aresta a = b->g->arestas[i];
    int extremos[] = {a.u, a.v};
    for (int ramo = 0; ramo < 2; ramo++) {
        int v = extremos[ramo];
        b->selecionados[v] = 1;
        b->parcial[quantidade] = v;
        buscar(b, quantidade + 1);
        b->selecionados[v] = 0;
    }
}

ResultadoCobertura cobertura_minima_bt(const Grafo *g) {
    ResultadoCobertura r = cobertura_gulosa(g);
    if (r.status != COBERTURA_SUCESSO || r.quantidade == 0) {
        return r;
    }
    unsigned char *selecionados = alocar(g->n, sizeof(*selecionados));
    int *parcial = alocar(g->n, sizeof(*parcial));
    if (selecionados == NULL || parcial == NULL) {
        free(selecionados);
        free(parcial);
        liberar_resultado_cobertura(&r);
        r.status = COBERTURA_SEM_MEMORIA;
        return r;
    }
    Busca b = {g, selecionados, parcial, &r};
    buscar(&b, 0);
    free(selecionados);
    free(parcial);
    return r;
}
