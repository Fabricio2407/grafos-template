#include "hamiltoniano.h"
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>

int grafo_valido(const Grafo *g) {
    if (!g || g->n < 0 || g->n == INT_MAX) return 0;
    size_t n = (size_t)g->n;
    if (n && (n > SIZE_MAX / n || !g->adj ||
              n + 1 > SIZE_MAX / sizeof(int))) return 0;
    for (int i = 0; i < g->n; i++) {
        if (g->adj[(size_t)i*n+i]) return 0;
        for (int j = i+1; j < g->n; j++) {
            unsigned char a = g->adj[(size_t)i*n+j];
            if (a > 1 || a != g->adj[(size_t)j*n+i]) return 0;
        }
    }
    return 1;
}

void liberar_resultado_hamiltoniano(ResultadoHamiltoniano *r) {
    if (r) {
        free(r->vertices);
        *r = (ResultadoHamiltoniano){.status = ENTRADA_INVALIDA};
    }
}

static int buscar(const Grafo *g, int *rota, unsigned char *usado,
                  int profundidade, int ciclo) {
    if (profundidade == g->n)
        return !ciclo || g->adj[(size_t)rota[g->n-1]*g->n+rota[0]];
    int ultimo = rota[profundidade-1];
    for (int v = 0; v < g->n; v++) {
        if (!usado[v] && g->adj[(size_t)ultimo*g->n+v]) {
            usado[v] = 1;
            rota[profundidade] = v;
            int achou = buscar(g, rota, usado, profundidade+1, ciclo);
            usado[v] = 0; /* restaura inclusive ao propagar sucesso */
            if (achou) return 1;
        }
    }
    return 0;
}

static ResultadoHamiltoniano resolver(const Grafo *g, int ciclo) {
    ResultadoHamiltoniano r = {.status = ENTRADA_INVALIDA};
    if (!grafo_valido(g)) return r;
    r.status = SEM_SOLUCAO;
    if (!g->n || (ciclo && g->n < 3)) return r;
    int *rota = malloc(((size_t)g->n+1)*sizeof(*rota));
    unsigned char *usado = calloc((size_t)g->n, 1);
    if (!rota || !usado) {
        free(rota); free(usado);
        r.status = SEM_MEMORIA;
        return r;
    }
    for (int origem = 0; origem < (ciclo ? 1 : g->n); origem++) {
        rota[0] = origem;
        usado[origem] = 1;
        int achou = buscar(g, rota, usado, 1, ciclo);
        usado[origem] = 0;
        if (achou) {
            if (ciclo) rota[g->n] = origem;
            r = (ResultadoHamiltoniano){SOLUCAO, rota, g->n+ciclo};
            break;
        }
    }
    free(usado);
    if (r.status != SOLUCAO) free(rota);
    return r;
}

ResultadoHamiltoniano encontrar_caminho_hamiltoniano_bt(const Grafo *g) {
    return resolver(g, 0);
}
ResultadoHamiltoniano encontrar_ciclo_hamiltoniano_bt(const Grafo *g) {
    return resolver(g, 1);
}
