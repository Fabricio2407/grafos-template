#include "tps.h"
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static int dimensao_valida(int n) {
    if (n < 0 || n == INT_MAX) return 0;
    size_t s = (size_t)n;
    return !s || (s <= SIZE_MAX/s && s*s <= SIZE_MAX/sizeof(int64_t) &&
                  s+1 <= SIZE_MAX/sizeof(int));
}

static int valido(const GrafoTSP *g) {
    if (!g || !dimensao_valida(g->n) || (g->n && !g->pesos)) return 0;
    for (int i = 0; i < g->n; i++) {
        if (g->pesos[(size_t)i*g->n+i]) return 0;
        for (int j = i+1; j < g->n; j++) {
            int64_t p = g->pesos[(size_t)i*g->n+j];
            if (p < 0 || p != g->pesos[(size_t)j*g->n+i]) return 0;
        }
    }
    return 1;
}

static int64_t peso(const GrafoTSP *g, int u, int v) {
    return g->pesos[(size_t)u*g->n+v];
}

/* Todos os operandos deste somatorio sao nao negativos. */
static int somar(int64_t a, int64_t b, int64_t *soma) {
    if (a > INT64_MAX-b) return 0;
    *soma = a+b;
    return 1;
}

void liberar_resultado_tsp(ResultadoTSP *r) {
    if (r) {
        free(r->vertices);
        *r = (ResultadoTSP){.status = ENTRADA_INVALIDA};
    }
}

void liberar_grafo_tsp(GrafoTSP *g) {
    if (g) {
        free(g->pesos);
        *g = (GrafoTSP){0};
    }
}

Status grafo_para_tsp(const Grafo *g, GrafoTSP *saida) {
    if (!saida) return ENTRADA_INVALIDA;
    *saida = (GrafoTSP){0};
    if (!grafo_valido(g) || !dimensao_valida(g->n)) return ENTRADA_INVALIDA;
    if (!g->n) return SOLUCAO;
    size_t n = (size_t)g->n;
    int64_t *p = calloc(n*n, sizeof(*p));
    if (!p) return SEM_MEMORIA;
    for (int i = 0; i < g->n; i++)
        for (int j = i+1; j < g->n; j++)
            p[(size_t)i*n+j] = p[(size_t)j*n+i] = g->adj[(size_t)i*n+j] ? 1 : 2;
    *saida = (GrafoTSP){g->n, p};
    return SOLUCAO;
}

static ResultadoTSP preparar(const GrafoTSP *g) {
    ResultadoTSP r = {.status = ENTRADA_INVALIDA};
    if (!valido(g)) return r;
    r.status = SEM_SOLUCAO;
    if (!g->n) return r;
    r.vertices = calloc((size_t)g->n+1, sizeof(*r.vertices));
    r.status = r.vertices ? SOLUCAO : SEM_MEMORIA;
    if (r.vertices) r.quantidade = g->n+1;
    return r;
}

static void falhar(ResultadoTSP *r, Status status) {
    uint64_t contagem = r->rotas_avaliadas;
    liberar_resultado_tsp(r);
    r->status = status;
    r->rotas_avaliadas = contagem;
}

typedef struct {
    const GrafoTSP *g;
    int *rota;
    unsigned char *usado;
    ResultadoTSP *melhor;
    int encontrado;
} BuscaTSP;

static void permutar(BuscaTSP *b, int profundidade, int64_t custo, int excedeu) {
    int n = b->g->n;
    if (profundidade == n) {
        b->melhor->rotas_avaliadas++;
        int64_t total;
        if (!excedeu && somar(custo, peso(b->g, b->rota[n-1], 0), &total) &&
            (!b->encontrado || total < b->melhor->custo)) {
            memcpy(b->melhor->vertices, b->rota, (size_t)n*sizeof(int));
            b->melhor->vertices[n] = 0;
            b->melhor->custo = total;
            b->encontrado = 1;
        }
        return;
    }
    for (int v = 1; v < n; v++) {
        if (!b->usado[v]) {
            int64_t novo = 0;
            int overflow = excedeu || !somar(custo,
                peso(b->g, b->rota[profundidade-1], v), &novo);
            b->rota[profundidade] = v;
            b->usado[v] = 1;
            /* Mesmo com overflow nao poda: conta todas as permutacoes. */
            permutar(b, profundidade+1, novo, overflow);
            b->usado[v] = 0;
        }
    }
}

ResultadoTSP tsp_forca_bruta(const GrafoTSP *g) {
    ResultadoTSP r = preparar(g);
    if (r.status != SOLUCAO) return r;
    uint64_t fatorial = 1;
    for (int i = 2; i < g->n; i++) {
        if (fatorial > UINT64_MAX/(uint64_t)i) {
            falhar(&r, OVERFLOW);
            return r;
        }
        fatorial *= (uint64_t)i;
    }
    int *rota = calloc((size_t)g->n, sizeof(*rota));
    unsigned char *usado = calloc((size_t)g->n, 1);
    if (!rota || !usado) {
        free(rota); free(usado);
        falhar(&r, SEM_MEMORIA);
        return r;
    }
    BuscaTSP b = {g, rota, usado, &r, 0};
    usado[0] = 1;
    permutar(&b, 1, 0, 0);
    free(rota); free(usado);
    if (!b.encontrado) falhar(&r, OVERFLOW);
    return r;
}

static void conferir_custo(const GrafoTSP *g, ResultadoTSP *r) {
    r->custo = 0;
    for (int i = 1; i < r->quantidade; i++) {
        if (!somar(r->custo, peso(g, r->vertices[i-1], r->vertices[i]), &r->custo)) {
            falhar(r, OVERFLOW);
            return;
        }
    }
}

ResultadoTSP tsp_vizinho_mais_proximo(const GrafoTSP *g) {
    ResultadoTSP r = preparar(g);
    if (r.status != SOLUCAO) return r;
    unsigned char *usado = calloc((size_t)g->n, 1);
    if (!usado) { falhar(&r, SEM_MEMORIA); return r; }
    usado[0] = 1;
    for (int i = 1; i < g->n; i++) {
        int melhor = -1;
        for (int v = 1; v < g->n; v++) {
            if (!usado[v] && (melhor == -1 ||
                peso(g, r.vertices[i-1], v) < peso(g, r.vertices[i-1], melhor)))
                melhor = v;
        }
        r.vertices[i] = melhor;
        usado[melhor] = 1;
    }
    free(usado);
    conferir_custo(g, &r);
    return r;
}

/* a+b pode chegar a 2*INT64_MAX, que cabe em uint64_t. A diferenca
 * pode exceder INT64_MAX positivamente; sinal/magnitude evitam overflow
 * e permitem comparar variacoes sem rejeitar uma insercao valida. */
typedef struct { int negativo; uint64_t magnitude; } Variacao;
static Variacao variacao(int64_t a, int64_t b, int64_t anterior) {
    uint64_t soma = (uint64_t)a+(uint64_t)b, c = (uint64_t)anterior;
    return soma < c ? (Variacao){1,c-soma} : (Variacao){0,soma-c};
}
static int menor(Variacao a, Variacao b) {
    if (a.negativo != b.negativo) return a.negativo;
    return a.negativo ? a.magnitude > b.magnitude : a.magnitude < b.magnitude;
}

ResultadoTSP tsp_insercao(const GrafoTSP *g) {
    ResultadoTSP r = preparar(g);
    if (r.status != SOLUCAO) return r;
    unsigned char *usado = calloc((size_t)g->n, 1);
    if (!usado) { falhar(&r, SEM_MEMORIA); return r; }
    usado[0] = 1;
    /* Rota inicial [0,0]; primeiro passo escolhe o mais proximo de 0. */
    for (int tamanho = 1; tamanho < g->n; tamanho++) {
        int escolhido = -1;
        int64_t distancia = 0;
        for (int v = 1; v < g->n; v++) {
            if (usado[v]) continue;
            int64_t d = peso(g, v, r.vertices[0]);
            for (int i = 1; i < tamanho; i++) {
                int64_t p = peso(g, v, r.vertices[i]);
                if (p < d) d = p;
            }
            if (escolhido == -1 || d < distancia) {
                escolhido = v;
                distancia = d;
            }
        }
        int posicao = 0;
        Variacao melhor = {0,0};
        for (int i = 0; i < tamanho; i++) {
            int a = r.vertices[i], b = r.vertices[i+1];
            Variacao delta = variacao(peso(g,a,escolhido),
                                     peso(g,escolhido,b), peso(g,a,b));
            if (i == 0 || menor(delta, melhor)) {
                melhor = delta;
                posicao = i;
            }
        }
        memmove(r.vertices+posicao+2, r.vertices+posicao+1,
                (size_t)(tamanho-posicao)*sizeof(int));
        r.vertices[posicao+1] = escolhido;
        usado[escolhido] = 1;
    }
    free(usado);
    conferir_custo(g, &r); /* soma novamente as arestas, sem usar deltas */
    return r;
}
