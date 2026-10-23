#include <assert.h>
#include <stdio.h>
#include "dijkstra.h"

static void conferir_heap(MinHeap *h) {
    int presentes = 0;
    for (int v = 0; v < h->capacidade; v++) {
        int i = h->posicao[v];
        if (i != -1) {
            assert(i >= 0 && i < h->tamanho);
            assert(h->dados[i].vertice == v);
            presentes++;
        }
    }
    assert(presentes == h->tamanho);
    for (int i = 0; i < h->tamanho; i++) {
        assert(h->posicao[h->dados[i].vertice] == i);
        if (i > 0) {
            assert(h->dados[(i - 1) / 2].distancia <= h->dados[i].distancia);
        }
    }
}

static void testar_heap(void) {
    assert(criar_min_heap(0) == NULL);
    MinHeap *h = criar_min_heap(8);
    assert(h != NULL);
    NoHeap no = {-7, -7};
    assert(!extrair_minimo(h, &no) && no.vertice == -7);
    assert(!inserir_min_heap(h, -1, 0));
    assert(!inserir_min_heap(h, 8, 0));
    for (int i = 0; i < 8; i++) {
        assert(inserir_min_heap(h, i, 100 - i));
        conferir_heap(h);
    }
    assert(!inserir_min_heap(h, 0, 0));
    assert(!diminuir_chave(h, 8, 0));
    assert(!diminuir_chave(h, 0, 100));
    assert(!diminuir_chave(h, 0, 101));
    assert(diminuir_chave(h, 0, 50));
    conferir_heap(h);
    assert(diminuir_chave(h, 0, 10));
    conferir_heap(h);
    int anterior = INT_MIN;
    while (extrair_minimo(h, &no)) {
        assert(no.distancia >= anterior);
        anterior = no.distancia;
        assert(h->posicao[no.vertice] == -1);
        assert(!diminuir_chave(h, no.vertice, 0));
        conferir_heap(h);
    }
    assert(inserir_min_heap(h, 0, 1));
    assert(!inserir_min_heap(h, 0, 0));
    conferir_heap(h);
    liberar_min_heap(h);
    liberar_min_heap(NULL);
}

static void conferir_falha(int *dist, int *pred, int n) {
    for (int i = 0; i < n; i++) {
        assert(dist[i] == INT_MAX && pred[i] == -1);
    }
}

static void testar_dijkstra(void) {
    int dist[5], pred[5];
    GrafoListaPonderado *g = criar_grafo_ponderado(5);
    assert(g != NULL);
    assert(inserir_aresta_ponderada(g, 0, 1, 4));
    assert(inserir_aresta_ponderada(g, 0, 2, 1));
    assert(inserir_aresta_ponderada(g, 1, 3, 1));
    assert(inserir_aresta_ponderada(g, 2, 1, 2));
    assert(inserir_aresta_ponderada(g, 2, 3, 5));
    dijkstra(g, 0, dist, pred);
    assert(dist[0] == 0 && dist[1] == 3 && dist[2] == 1 && dist[3] == 4);
    assert(dist[4] == INT_MAX && pred[4] == -1);
    assert(pred[3] == 1 && pred[1] == 2 && pred[2] == 0 && pred[0] == -1);
    reconstruir_caminho(0, 3, pred);
    reconstruir_caminho(0, 4, pred);
    reconstruir_caminho(0, 0, pred);
    assert(!inserir_aresta_ponderada(g, 4, 0, -1));
    assert(inserir_aresta_ponderada(g, 4, 0, 0));
    /* Simula grafo construido externamente com peso negativo inalcançavel. */
    g->adj[4]->peso = -1;
    dijkstra(g, 0, dist, pred);
    conferir_falha(dist, pred, 5);
    g->adj[4]->peso = 0;
    dijkstra(g, 5, dist, pred);
    conferir_falha(dist, pred, 5);
    dijkstra(g, -1, dist, pred);
    conferir_falha(dist, pred, 5);
    dijkstra(g, 0, NULL, pred);
    for (int i = 0; i < 5; i++) {
        assert(pred[i] == -1);
    }
    liberar_grafo_ponderado(g);

    g = criar_grafo_ponderado(1);
    assert(g != NULL);
    assert(inserir_aresta_ponderada(g, 0, 0, 0));
    dijkstra(g, 0, dist, pred);
    assert(dist[0] == 0 && pred[0] == -1);
    liberar_grafo_ponderado(g);

    g = criar_grafo_ponderado(3);
    assert(g != NULL);
    assert(inserir_aresta_ponderada(g, 0, 1, 0));
    assert(inserir_aresta_ponderada(g, 1, 2, 0));
    assert(inserir_aresta_ponderada(g, 2, 0, 0));
    dijkstra(g, 0, dist, pred);
    assert(dist[0] == 0 && dist[1] == 0 && dist[2] == 0);
    assert(pred[0] == -1 && pred[1] == 0 && pred[2] == 1);
    g->adj[0]->peso = INT_MAX - 1;
    dijkstra(g, 0, dist, pred);
    assert(dist[1] == INT_MAX - 1 && dist[2] == INT_MAX - 1);
    g->adj[1]->peso = 1;
    dijkstra(g, 0, dist, pred);
    conferir_falha(dist, pred, 3);
    g->adj[1]->peso = 2;
    dijkstra(g, 0, dist, pred);
    conferir_falha(dist, pred, 3);
    g->adj[0]->peso = INT_MAX;
    dijkstra(g, 0, dist, pred);
    conferir_falha(dist, pred, 3);
    liberar_grafo_ponderado(g);
    dijkstra(NULL, 0, NULL, NULL);
}

int main(void) {
    testar_heap();
    testar_dijkstra();
    puts("Todos os testes passaram.");
    return 0;
}
