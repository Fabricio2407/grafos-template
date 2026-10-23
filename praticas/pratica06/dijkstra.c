#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "dijkstra.h"

static void trocar(MinHeap *h, int a, int b) {
    NoHeap aux = h->dados[a];
    h->dados[a] = h->dados[b];
    h->dados[b] = aux;
    h->posicao[h->dados[a].vertice] = a;
    h->posicao[h->dados[b].vertice] = b;
}

static void subir(MinHeap *h, int i) {
    while (i > 0) {
        int pai = (i - 1) / 2;
        if (h->dados[pai].distancia <= h->dados[i].distancia) {
            break;
        }
        trocar(h, pai, i);
        i = pai;
    }
}

MinHeap *criar_min_heap(int capacidade) {
    if (capacidade <= 0 || (size_t)capacidade > SIZE_MAX / sizeof(NoHeap) ||
        (size_t)capacidade > SIZE_MAX / sizeof(int)) {
        return NULL;
    }
    MinHeap *h = malloc(sizeof(*h));
    if (h == NULL) {
        return NULL;
    }
    h->dados = malloc((size_t)capacidade * sizeof(*h->dados));
    h->posicao = malloc((size_t)capacidade * sizeof(*h->posicao));
    if (h->dados == NULL || h->posicao == NULL) {
        liberar_min_heap(h);
        return NULL;
    }
    h->capacidade = capacidade;
    h->tamanho = 0;
    for (int v = 0; v < capacidade; v++) {
        h->posicao[v] = -1;
    }
    return h;
}

int inserir_min_heap(MinHeap *h, int vertice, int distancia) {
    if (h == NULL || vertice < 0 || vertice >= h->capacidade ||
        h->tamanho >= h->capacidade || h->posicao[vertice] != -1) {
        return 0;
    }
    int i = h->tamanho++;
    h->dados[i] = (NoHeap){vertice, distancia};
    h->posicao[vertice] = i;
    subir(h, i);
    return 1;
}

int diminuir_chave(MinHeap *h, int vertice, int distancia) {
    if (h == NULL || vertice < 0 || vertice >= h->capacidade) {
        return 0;
    }
    int i = h->posicao[vertice];
    if (i < 0 || i >= h->tamanho || distancia >= h->dados[i].distancia) {
        return 0;
    }
    h->dados[i].distancia = distancia;
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
    /* A condicao garante que 2*i+1 existe e nao causa overflow. */
    while (i < h->tamanho / 2) {
        int menor = 2 * i + 1;
        int direito = menor + 1;
        if (direito < h->tamanho &&
            h->dados[direito].distancia < h->dados[menor].distancia) {
            menor = direito;
        }
        if (h->dados[i].distancia <= h->dados[menor].distancia) {
            break;
        }
        trocar(h, i, menor);
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
    if (peso < 0) {
        fprintf(stderr, "Aresta rejeitada: peso negativo.\n");
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
    for (int u = 0; u < g->n; u++) {
        NoPonderado *no = g->adj[u];
        while (no != NULL) {
            NoPonderado *prox = no->prox;
            free(no);
            no = prox;
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

void dijkstra(GrafoListaPonderado *g, int origem, int *dist, int *pred) {
    MinHeap *h = NULL;
    const char *erro = "argumentos ou origem invalidos";
    if (g == NULL || g->n <= 0) {
        fprintf(stderr, "Dijkstra: %s.\n", erro);
        return;
    }
    limpar_resultados(g->n, dist, pred);
    if (g->adj == NULL || dist == NULL || pred == NULL || dist == pred ||
        origem < 0 || origem >= g->n) {
        goto falha;
    }
    /* Validar todo o grafo antes de calcular, inclusive componentes isolados. */
    for (int u = 0; u < g->n; u++) {
        for (NoPonderado *no = g->adj[u]; no != NULL; no = no->prox) {
            if (no->peso < 0) {
                erro = "peso negativo nao permitido";
                goto falha;
            }
            if (no->destino < 0 || no->destino >= g->n) {
                erro = "destino de aresta invalido";
                goto falha;
            }
        }
    }
    h = criar_min_heap(g->n);
    if (h == NULL) {
        erro = "falha de alocacao do heap";
        goto falha;
    }
    /* Todos entram uma vez. posicao == -1 passa a significar finalizado. */
    for (int v = 0; v < g->n; v++) {
        inserir_min_heap(h, v, INT_MAX);
    }
    dist[origem] = 0;
    diminuir_chave(h, origem, 0);
    NoHeap atual;
    while (extrair_minimo(h, &atual)) {
        if (atual.distancia == INT_MAX) {
            break;
        }
        int u = atual.vertice;
        for (NoPonderado *no = g->adj[u]; no != NULL; no = no->prox) {
            int v = no->destino;
            if (h->posicao[v] == -1) {
                continue;
            }
            if (no->peso >= INT_MAX - dist[u]) {
                erro = "soma de custos >= INT_MAX; resultados invalidados";
                goto falha;
            }
            int nova = dist[u] + no->peso;
            if (nova < dist[v]) {
                diminuir_chave(h, v, nova);
                dist[v] = nova;
                pred[v] = u;
            }
        }
    }
    liberar_min_heap(h);
    return;

falha:
    liberar_min_heap(h);
    limpar_resultados(g->n, dist, pred);
    fprintf(stderr, "Dijkstra: %s.\n", erro);
}

void reconstruir_caminho(int origem, int destino, int *pred) {
    if (pred == NULL || origem < 0 || destino < 0) {
        fprintf(stderr, "Reconstrucao: argumentos invalidos.\n");
        return;
    }
    size_t tamanho = 1;
    int v = destino;
    while (v != origem) {
        v = pred[v];
        if (v == -1) {
            printf("Nao existe caminho de %d ate %d.\n", origem, destino);
            return;
        }
        if (tamanho == SIZE_MAX / sizeof(int)) {
            fprintf(stderr, "Reconstrucao: caminho muito grande.\n");
            return;
        }
        tamanho++;
    }
    int *caminho = malloc(tamanho * sizeof(*caminho));
    if (caminho == NULL) {
        fprintf(stderr, "Reconstrucao: falha de alocacao.\n");
        return;
    }
    v = destino;
    for (size_t i = tamanho; i > 0; i--) {
        caminho[i - 1] = v;
        if (i > 1) {
            v = pred[v];
        }
    }
    for (size_t i = 0; i < tamanho; i++) {
        printf("%s%d", i == 0 ? "" : " -> ", caminho[i]);
    }
    printf("\n");
    free(caminho);
}
