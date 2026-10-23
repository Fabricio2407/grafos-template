#include <stdint.h>
#include <stdlib.h>

#include "bellman_ford.h"

GrafoListaPonderado *criar_grafo_ponderado(int n) {
    if (n <= 0 || (size_t)n > SIZE_MAX / sizeof(NoPonderado *)) {
        return NULL;
    }
    GrafoListaPonderado *g = malloc(sizeof(*g));
    if (g == NULL) {
        return NULL;
    }
    g->n = n;
    g->adj = calloc((size_t)n, sizeof(*g->adj));
    if (g->adj == NULL) {
        free(g);
        return NULL;
    }
    return g;
}

int inserir_aresta_ponderada(GrafoListaPonderado *g, int u, int v, int peso) {
    if (g == NULL || g->adj == NULL || u < 0 || u >= g->n ||
        v < 0 || v >= g->n) {
        return 0;
    }
    NoPonderado *no = malloc(sizeof(*no));
    if (no == NULL) {
        return 0;
    }
    *no = (NoPonderado){v, peso, g->adj[u]};
    g->adj[u] = no;
    return 1;
}

void liberar_grafo_ponderado(GrafoListaPonderado *g) {
    if (g == NULL) {
        return;
    }
    if (g->adj != NULL) {
        for (int u = 0; u < g->n; u++) {
            NoPonderado *no = g->adj[u];
            while (no != NULL) {
                NoPonderado *prox = no->prox;
                free(no);
                no = prox;
            }
        }
    }
    free(g->adj);
    free(g);
}

static void limpar_resultados(int n, int *dist, int *pred) {
    for (int v = 0; v < n; v++) {
        if (dist != NULL) {
            dist[v] = INT_MAX;
        }
        if (pred != NULL) {
            pred[v] = -1;
        }
    }
}

/* Retorna -1 se a soma nao couber em int64_t; senao, 1.
 * Alcancabilidade separada permite usar todo o intervalo interno. */
static int relaxar_rodada(GrafoListaPonderado *g, int64_t *custos,
                         unsigned char *alcancavel, int *pred, int *ultimo) {
    *ultimo = -1;
    for (int u = 0; u < g->n; u++) {
        for (NoPonderado *no = g->adj[u]; no != NULL; no = no->prox) {
            if (!alcancavel[u]) {
                continue;
            }
            int64_t peso = no->peso;
            if ((peso > 0 && custos[u] > INT64_MAX - peso) ||
                (peso < 0 && custos[u] < INT64_MIN - peso)) {
                return -1;
            }
            int64_t nova = custos[u] + peso;
            int v = no->destino;
            if (!alcancavel[v] || nova < custos[v]) {
                custos[v] = nova;
                alcancavel[v] = 1;
                pred[v] = u;
                *ultimo = v;
            }
        }
    }
    return 1;
}

int bellman_ford_com_testemunha(GrafoListaPonderado *g, int origem,
                              int *dist, int *pred, int *testemunha) {
    int64_t *custos = NULL;
    unsigned char *alcancavel = NULL;
    const char *erro = "argumentos ou origem invalidos";
    if (testemunha != NULL) {
        *testemunha = -1;
    }
    if (g == NULL || g->n <= 0) {
        fprintf(stderr, "Bellman-Ford: %s.\n", erro);
        return -1;
    }
    limpar_resultados(g->n, dist, pred);
    if (g->adj == NULL || dist == NULL || pred == NULL || dist == pred ||
        testemunha == NULL || origem < 0 || origem >= g->n) {
        goto falha;
    }
    for (int u = 0; u < g->n; u++) {
        for (NoPonderado *no = g->adj[u]; no != NULL; no = no->prox) {
            if (no->destino < 0 || no->destino >= g->n) {
                erro = "destino de aresta invalido";
                goto falha;
            }
        }
    }
    if ((size_t)g->n > SIZE_MAX / sizeof(*custos)) {
        erro = "tamanho de alocacao nao representavel";
        goto falha;
    }
    custos = calloc((size_t)g->n, sizeof(*custos));
    alcancavel = calloc((size_t)g->n, sizeof(*alcancavel));
    if (custos == NULL || alcancavel == NULL) {
        erro = "falha de alocacao";
        goto falha;
    }
    custos[origem] = 0;
    alcancavel[origem] = 1;
    dist[origem] = 0;
    int ultimo = -1;
    erro = "soma fora do intervalo de int64_t";
    for (int rodada = 0; rodada < g->n - 1; rodada++) {
        if (relaxar_rodada(g, custos, alcancavel, pred, &ultimo) == -1) {
            goto falha;
        }
        if (ultimo == -1) {
            break;
        }
    }
    /* Rodada n: atualiza tambem pred e registra uma testemunha do ciclo. */
    if (relaxar_rodada(g, custos, alcancavel, pred, &ultimo) == -1) {
        goto falha;
    }
    if (ultimo != -1) {
        *testemunha = ultimo;
        for (int v = 0; v < g->n; v++) {
            dist[v] = INT_MAX;
        }
        free(custos);
        free(alcancavel);
        return 0;
    }
    /* Validar TODOS os custos antes de converter qualquer resultado. */
    for (int v = 0; v < g->n; v++) {
        if (alcancavel[v] && (custos[v] < INT_MIN || custos[v] >= INT_MAX)) {
            erro = "custo nao representavel em int (INT_MAX reservado)";
            goto falha;
        }
    }
    for (int v = 0; v < g->n; v++) {
        if (alcancavel[v]) {
            dist[v] = (int)custos[v];
        }
    }
    free(custos);
    free(alcancavel);
    return 1;

falha:
    free(custos);
    free(alcancavel);
    limpar_resultados(g->n, dist, pred);
    fprintf(stderr, "Bellman-Ford: %s.\n", erro);
    return -1;
}

int bellman_ford(GrafoListaPonderado *g, int origem, int *dist, int *pred) {
    int testemunha;
    return bellman_ford_com_testemunha(g, origem, dist, pred, &testemunha);
}

static int predecessor_valido(int n, const int *pred, int v) {
    return v >= 0 && v < n && pred[v] >= 0 && pred[v] < n;
}

int reconstruir_ciclo_negativo(int n, const int *pred, int testemunha,
                              FILE *saida) {
    int *ciclo = NULL;
    const char *erro = "dados invalidos para reconstruir ciclo";
    if (n <= 0 || pred == NULL || saida == NULL ||
        testemunha < 0 || testemunha >= n) {
        goto falha;
    }
    int v = testemunha;
    for (int i = 0; i < n; i++) {
        if (!predecessor_valido(n, pred, v)) {
            goto falha;
        }
        v = pred[v];
    }
    if ((size_t)n > SIZE_MAX / sizeof(*ciclo)) {
        erro = "tamanho de alocacao nao representavel";
        goto falha;
    }
    ciclo = malloc((size_t)n * sizeof(*ciclo));
    if (ciclo == NULL) {
        erro = "falha de alocacao do ciclo";
        goto falha;
    }
    int inicio = v;
    int tamanho = 0;
    do {
        if (tamanho == n || !predecessor_valido(n, pred, v)) {
            goto falha;
        }
        ciclo[tamanho++] = v;
        v = pred[v];
    } while (v != inicio);

    /* Predecessores percorrem as arestas ao contrario: inverter ao imprimir. */
    for (int i = tamanho - 1; i >= 0; i--) {
        if (fprintf(saida, "%d -> ", ciclo[i]) < 0) {
            erro = "falha ao escrever ciclo";
            goto falha;
        }
    }
    if (fprintf(saida, "%d\n", ciclo[tamanho - 1]) < 0) {
        erro = "falha ao escrever ciclo";
        goto falha;
    }
    free(ciclo);
    return 1;

falha:
    free(ciclo);
    fprintf(stderr, "Reconstrucao: %s.\n", erro);
    return -1;
}
