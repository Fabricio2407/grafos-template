#include <stddef.h>
#include <stdlib.h>

#include "agm.h"

struct GrafoListaPonderado {
    int n, m;
    NoPonderado **adj;
    Aresta *arestas;
};

static void *alocar_vetor(int n, size_t tamanho) {
    if (n <= 0 || (size_t)n > SIZE_MAX / tamanho) {
        return NULL;
    }
    return calloc((size_t)n, tamanho);
}

DSU *criar_dsu(int n) {
    if (n < 0) {
        return NULL;
    }
    DSU *d = calloc(1, sizeof(*d));
    if (d == NULL) {
        return NULL;
    }
    d->n = n;
    if (n > 0) {
        d->pai = alocar_vetor(n, sizeof(*d->pai));
        d->rank = alocar_vetor(n, sizeof(*d->rank));
        if (d->pai == NULL || d->rank == NULL) {
            liberar_dsu(d);
            return NULL;
        }
        for (int i = 0; i < n; i++) {
            d->pai[i] = i;
        }
    }
    return d;
}

int find(DSU *d, int x) {
    if (d == NULL || x < 0 || x >= d->n || d->pai == NULL) {
        return -1;
    }
    int raiz = x;
    while (d->pai[raiz] != raiz) {
        raiz = d->pai[raiz];
    }
    while (x != raiz) {
        int proximo = d->pai[x];
        d->pai[x] = raiz;
        x = proximo;
    }
    return raiz;
}

int unir(DSU *d, int x, int y) {
    if (d == NULL || d->rank == NULL) {
        return -1;
    }
    int a = find(d, x);
    int b = find(d, y);
    if (a == -1 || b == -1) {
        return -1;
    }
    if (a == b) {
        return 0;
    }
    if (d->rank[a] < d->rank[b]) {
        d->pai[a] = b;
    } else {
        d->pai[b] = a;
        if (d->rank[a] == d->rank[b]) {
            d->rank[a]++;
        }
    }
    return 1;
}

void liberar_dsu(DSU *d) {
    if (d != NULL) {
        free(d->pai);
        free(d->rank);
        free(d);
    }
}

static void trocar_heap(MinHeap *h, int a, int b) {
    NoHeap aux = h->dados[a];
    h->dados[a] = h->dados[b];
    h->dados[b] = aux;
    h->posicao[h->dados[a].vertice] = a;
    h->posicao[h->dados[b].vertice] = b;
}

static void subir(MinHeap *h, int i) {
    while (i > 0) {
        int pai = (i - 1) / 2;
        if (h->dados[pai].chave <= h->dados[i].chave) {
            break;
        }
        trocar_heap(h, pai, i);
        i = pai;
    }
}

MinHeap *criar_min_heap(int capacidade) {
    if (capacidade <= 0) {
        return NULL;
    }
    MinHeap *h = calloc(1, sizeof(*h));
    if (h == NULL) {
        return NULL;
    }
    h->capacidade = capacidade;
    h->dados = alocar_vetor(capacidade, sizeof(*h->dados));
    h->posicao = alocar_vetor(capacidade, sizeof(*h->posicao));
    if (h->dados == NULL || h->posicao == NULL) {
        liberar_min_heap(h);
        return NULL;
    }
    for (int v = 0; v < capacidade; v++) {
        h->posicao[v] = -1;
    }
    return h;
}

int inserir_min_heap(MinHeap *h, int vertice, int64_t chave) {
    if (h == NULL || vertice < 0 || vertice >= h->capacidade ||
        h->tamanho == h->capacidade || h->posicao[vertice] != -1) {
        return 0;
    }
    int i = h->tamanho++;
    h->dados[i] = (NoHeap){vertice, chave};
    h->posicao[vertice] = i;
    subir(h, i);
    return 1;
}

int diminuir_chave(MinHeap *h, int vertice, int64_t chave) {
    if (h == NULL || vertice < 0 || vertice >= h->capacidade) {
        return 0;
    }
    int i = h->posicao[vertice];
    if (i < 0 || i >= h->tamanho || chave >= h->dados[i].chave) {
        return 0;
    }
    h->dados[i].chave = chave;
    subir(h, i);
    return 1;
}

int extrair_minimo(MinHeap *h, NoHeap *minimo) {
    if (h == NULL || minimo == NULL || h->tamanho == 0) {
        return 0;
    }
    *minimo = h->dados[0];
    h->posicao[minimo->vertice] = -1;
    h->tamanho--;
    if (h->tamanho == 0) {
        return 1;
    }
    h->dados[0] = h->dados[h->tamanho];
    h->posicao[h->dados[0].vertice] = 0;
    int i = 0;
    while (i < h->tamanho / 2) {
        int menor = 2 * i + 1;
        int direito = menor + 1;
        if (direito < h->tamanho && h->dados[direito].chave < h->dados[menor].chave) {
            menor = direito;
        }
        if (h->dados[i].chave <= h->dados[menor].chave) {
            break;
        }
        trocar_heap(h, i, menor);
        i = menor;
    }
    return 1;
}

void liberar_min_heap(MinHeap *h) {
    if (h != NULL) {
        free(h->dados);
        free(h->posicao);
        free(h);
    }
}

GrafoListaPonderado *criar_grafo_ponderado(int n, const Aresta *arestas, int m) {
    if (n < 0 || m < 0 || (m > 0 && arestas == NULL)) {
        return NULL;
    }
    for (int i = 0; i < m; i++) {
        if (arestas[i].u < 0 || arestas[i].u >= n ||
            arestas[i].v < 0 || arestas[i].v >= n) {
            return NULL;
        }
    }
    GrafoListaPonderado *g = calloc(1, sizeof(*g));
    if (g == NULL) {
        return NULL;
    }
    g->n = n;
    g->m = m;
    if (n > 0) {
        g->adj = alocar_vetor(n, sizeof(*g->adj));
        if (g->adj == NULL) {
            liberar_grafo_ponderado(g);
            return NULL;
        }
    }
    if (m > 0) {
        g->arestas = alocar_vetor(m, sizeof(*g->arestas));
        if (g->arestas == NULL) {
            liberar_grafo_ponderado(g);
            return NULL;
        }
    }
    for (int i = 0; i < m; i++) {
        Aresta a = arestas[i];
        NoPonderado *ida = malloc(sizeof(*ida));
        NoPonderado *volta = malloc(sizeof(*volta));
        if (ida == NULL || volta == NULL) {
            free(ida);
            free(volta);
            liberar_grafo_ponderado(g);
            return NULL;
        }
        g->arestas[i] = a;
        *ida = (NoPonderado){a.v, a.peso, g->adj[a.u]};
        g->adj[a.u] = ida;
        *volta = (NoPonderado){a.u, a.peso, g->adj[a.v]};
        g->adj[a.v] = volta;
    }
    return g;
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
    free(g->arestas);
    free(g);
}

void liberar_resultado_agm(ResultadoAGM *r) {
    if (r != NULL) {
        free(r->arestas);
        *r = (ResultadoAGM){.status = AGM_ENTRADA_INVALIDA};
    }
}

static ResultadoAGM resultado_erro(ResultadoAGM *r, StatusAGM status) {
    liberar_resultado_agm(r);
    r->status = status;
    return *r;
}

static int somar_peso(ResultadoAGM *r, Aresta a) {
    int64_t peso = a.peso;
    if ((peso > 0 && r->custo_total > INT64_MAX - peso) ||
        (peso < 0 && r->custo_total < INT64_MIN - peso)) {
        return 0;
    }
    r->custo_total += peso;
    r->arestas[r->quantidade++] = a;
    return 1;
}

/* Heap maximo de arestas para heapsort in-place; nenhuma subtracao de pesos. */
static void descer_arestas(Aresta *a, int tamanho, int i) {
    while (i < tamanho / 2) {
        int maior = 2 * i + 1;
        int direito = maior + 1;
        if (direito < tamanho && a[direito].peso > a[maior].peso) {
            maior = direito;
        }
        if (a[i].peso >= a[maior].peso) {
            break;
        }
        Aresta aux = a[i];
        a[i] = a[maior];
        a[maior] = aux;
        i = maior;
    }
}

static void ordenar_arestas(Aresta *a, int n) {
    for (int i = n / 2; i > 0; i--) {
        descer_arestas(a, n, i - 1);
    }
    for (int fim = n - 1; fim > 0; fim--) {
        Aresta aux = a[0];
        a[0] = a[fim];
        a[fim] = aux;
        descer_arestas(a, fim, 0);
    }
}

ResultadoAGM kruskal(const GrafoListaPonderado *g) {
    ResultadoAGM r = {.status = AGM_ENTRADA_INVALIDA};
    if (g == NULL) {
        return r;
    }
    r.status = AGM_SUCESSO;
    r.componentes = g->n;
    if (g->n <= 1 || g->m == 0) {
        return r;
    }
    r.arestas = alocar_vetor(g->n - 1, sizeof(*r.arestas));
    Aresta *ordenadas = alocar_vetor(g->m, sizeof(*ordenadas));
    DSU *d = criar_dsu(g->n);
    if (r.arestas == NULL || ordenadas == NULL || d == NULL) {
        free(ordenadas);
        liberar_dsu(d);
        return resultado_erro(&r, AGM_SEM_MEMORIA);
    }
    for (int i = 0; i < g->m; i++) {
        ordenadas[i] = g->arestas[i];
    }
    ordenar_arestas(ordenadas, g->m);
    for (int i = 0; i < g->m && r.quantidade < g->n - 1; i++) {
        Aresta a = ordenadas[i];
        if (a.u != a.v && unir(d, a.u, a.v) == 1) {
            if (!somar_peso(&r, a)) {
                free(ordenadas);
                liberar_dsu(d);
                return resultado_erro(&r, AGM_OVERFLOW);
            }
            r.componentes--;
        }
    }
    free(ordenadas);
    liberar_dsu(d);
    return r;
}

ResultadoAGM prim(const GrafoListaPonderado *g) {
    ResultadoAGM r = {.status = AGM_ENTRADA_INVALIDA};
    if (g == NULL) {
        return r;
    }
    r.status = AGM_SUCESSO;
    if (g->n <= 1 || g->m == 0) {
        r.componentes = g->n;
        return r;
    }
    r.arestas = alocar_vetor(g->n - 1, sizeof(*r.arestas));
    int *pred = alocar_vetor(g->n, sizeof(*pred));
    int64_t *chave = alocar_vetor(g->n, sizeof(*chave));
    MinHeap *h = criar_min_heap(g->n);
    if (r.arestas == NULL || pred == NULL || chave == NULL || h == NULL) {
        free(pred);
        free(chave);
        liberar_min_heap(h);
        return resultado_erro(&r, AGM_SEM_MEMORIA);
    }
    for (int v = 0; v < g->n; v++) {
        pred[v] = -1;
        /* Maior que qualquer peso int; nao conflita com INT_MAX. */
        chave[v] = INT64_MAX;
        inserir_min_heap(h, v, chave[v]);
    }
    NoHeap atual;
    while (extrair_minimo(h, &atual)) {
        int u = atual.vertice;
        if (pred[u] == -1) {
            /* Uma chave infinita inicia outra componente, inclusive isolada. */
            r.componentes++;
            chave[u] = 0;
        } else if (!somar_peso(&r, (Aresta){pred[u], u, (int)chave[u]})) {
            free(pred);
            free(chave);
            liberar_min_heap(h);
            return resultado_erro(&r, AGM_OVERFLOW);
        }
        for (NoPonderado *no = g->adj[u]; no != NULL; no = no->prox) {
            int v = no->destino;
            if (h->posicao[v] != -1 && (int64_t)no->peso < chave[v]) {
                chave[v] = no->peso;
                pred[v] = u;
                diminuir_chave(h, v, chave[v]);
            }
        }
    }
    free(pred);
    free(chave);
    liberar_min_heap(h);
    return r;
}
