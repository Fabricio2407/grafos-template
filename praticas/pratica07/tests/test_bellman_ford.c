#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "bellman_ford.h"

static GrafoListaPonderado *montar(int n, const int arestas[][3], int qtd) {
    GrafoListaPonderado *g = criar_grafo_ponderado(n);
    assert(g != NULL);
    for (int i = 0; i < qtd; i++) {
        assert(inserir_aresta_ponderada(g, arestas[i][0], arestas[i][1],
                                      arestas[i][2]));
    }
    return g;
}

/* Le a propria saida da reconstrucao: valida fechamento, sentido e custo. */
static void conferir_ciclo(GrafoListaPonderado *g, int *pred, int testemunha,
                          int64_t custo_esperado) {
    FILE *saida = tmpfile();
    assert(saida != NULL);
    assert(reconstruir_ciclo_negativo(g->n, pred, testemunha, saida) == 1);
    rewind(saida);
    int inicio, u, v;
    assert(fscanf(saida, "%d", &inicio) == 1);
    assert(inicio >= 0 && inicio < g->n);
    u = inicio;
    int quantidade = 0;
    int64_t custo = 0;
    unsigned char *visitado = calloc((size_t)g->n, sizeof(*visitado));
    assert(visitado != NULL);
    while (fscanf(saida, " -> %d", &v) == 1) {
        assert(v >= 0 && v < g->n);
        assert(!visitado[u]);
        visitado[u] = 1;
        NoPonderado *no = g->adj[u];
        while (no != NULL && no->destino != v) {
            no = no->prox;
        }
        assert(no != NULL);
        custo += no->peso;
        u = v;
        quantidade++;
        assert(quantidade <= g->n);
    }
    assert(quantidade > 0 && u == inicio);
    assert(custo < 0 && custo == custo_esperado);
    free(visitado);
    assert(fclose(saida) == 0);
}

static void conferir_falha(int n, int *dist, int *pred) {
    for (int i = 0; i < n; i++) {
        assert(dist[i] == INT_MAX && pred[i] == -1);
    }
}

static void testar_cenarios(void) {
    int dist[5], pred[5], testemunha;
    const int a[][3] = {{0, 1, 4}, {0, 2, 5}, {1, 3, 2},
                        {2, 1, -3}, {2, 3, 4}};
    GrafoListaPonderado *g = montar(5, a, 5);
    assert(bellman_ford_com_testemunha(g, 0, dist, pred, &testemunha) == 1);
    assert(testemunha == -1);
    const int esperado[] = {0, 2, 5, 4, INT_MAX};
    const int predecessores[] = {-1, 2, 0, 1, -1};
    for (int i = 0; i < 5; i++) {
        assert(dist[i] == esperado[i] && pred[i] == predecessores[i]);
    }
    liberar_grafo_ponderado(g);

    const int b[][3] = {{0, 1, 1}, {1, 2, -2}, {2, 3, -2}, {3, 1, 1}};
    g = montar(4, b, 4);
    assert(bellman_ford_com_testemunha(g, 0, dist, pred, &testemunha) == 0);
    conferir_ciclo(g, pred, testemunha, -3);
    for (int i = 0; i < 4; i++) {
        assert(dist[i] == INT_MAX);
    }
    assert(bellman_ford(g, 0, dist, pred) == 0);
    liberar_grafo_ponderado(g);

    const int c[][3] = {{0, 1, 2}, {2, 3, -2}, {3, 2, 1}};
    g = montar(4, c, 3);
    assert(bellman_ford_com_testemunha(g, 0, dist, pred, &testemunha) == 1);
    assert(testemunha == -1 && dist[0] == 0 && dist[1] == 2);
    assert(dist[2] == INT_MAX && dist[3] == INT_MAX);
    assert(pred[2] == -1 && pred[3] == -1);
    liberar_grafo_ponderado(g);
}

static void testar_casos_limite(void) {
    int dist[5], pred[5], testemunha;
    GrafoListaPonderado *g = criar_grafo_ponderado(1);
    assert(g != NULL);
    assert(bellman_ford(g, 0, dist, pred) == 1);
    assert(dist[0] == 0 && pred[0] == -1);
    assert(inserir_aresta_ponderada(g, 0, 0, INT_MIN));
    assert(bellman_ford_com_testemunha(g, 0, dist, pred, &testemunha) == 0);
    conferir_ciclo(g, pred, testemunha, INT_MIN);
    liberar_grafo_ponderado(g);

    const int zero[][3] = {{0, 1, 0}, {1, 2, -2}, {2, 1, 2}};
    g = montar(3, zero, 3);
    assert(bellman_ford(g, 0, dist, pred) == 1);
    assert(dist[0] == 0 && dist[1] == 0 && dist[2] == -2);
    liberar_grafo_ponderado(g);

    /* Ordem contraria dos vertices exige n-1 rodadas para chegar a 0. */
    const int cadeia[][3] = {{4, 3, 1}, {3, 2, 1}, {2, 1, 1}, {1, 0, 1}};
    g = montar(5, cadeia, 4);
    assert(bellman_ford(g, 4, dist, pred) == 1);
    for (int i = 0; i < 5; i++) {
        assert(dist[i] == 4 - i);
    }
    liberar_grafo_ponderado(g);

    /* Testemunha pode estar numa cauda que sai do ciclo. */
    const int cauda[][3] = {{0, 1, 0}, {1, 2, -3}, {2, 1, 1},
                            {2, 3, 0}, {3, 4, 0}};
    g = montar(5, cauda, 5);
    assert(bellman_ford_com_testemunha(g, 0, dist, pred, &testemunha) == 0);
    assert(testemunha == 4);
    conferir_ciclo(g, pred, testemunha, -2);
    liberar_grafo_ponderado(g);

    const int numerico[][3] = {{0, 1, INT_MAX - 1}, {1, 2, 0}};
    g = montar(3, numerico, 2);
    assert(bellman_ford(g, 0, dist, pred) == 1);
    assert(dist[2] == INT_MAX - 1);
    g->adj[1]->peso = 1;
    assert(bellman_ford(g, 0, dist, pred) == -1);
    conferir_falha(3, dist, pred);
    g->adj[1]->peso = 2;
    assert(bellman_ford(g, 0, dist, pred) == -1);
    conferir_falha(3, dist, pred);
    g->adj[0]->peso = INT_MIN;
    g->adj[1]->peso = 0;
    assert(bellman_ford(g, 0, dist, pred) == 1 && dist[2] == INT_MIN);
    g->adj[1]->peso = -1;
    assert(bellman_ford(g, 0, dist, pred) == -1);
    conferir_falha(3, dist, pred);
    liberar_grafo_ponderado(g);

    /* Valor temporario fora de int nao deve provocar falha prematura. */
    const int temporario[][3] = {{0, 1, INT_MAX}, {0, 2, 1}, {2, 1, 1}};
    g = montar(3, temporario, 3);
    assert(bellman_ford(g, 0, dist, pred) == 1 && dist[1] == 2);
    liberar_grafo_ponderado(g);

    const int muito_negativo[][3] = {{0, 1, INT_MIN}, {1, 0, INT_MIN}};
    g = montar(2, muito_negativo, 2);
    assert(bellman_ford_com_testemunha(g, 0, dist, pred, &testemunha) == 0);
    conferir_ciclo(g, pred, testemunha, (int64_t)INT_MIN * 2);
    liberar_grafo_ponderado(g);
}

static void testar_invalidos(void) {
    int dist[3], pred[3], testemunha = 99;
    assert(criar_grafo_ponderado(0) == NULL);
    assert(criar_grafo_ponderado(-1) == NULL);
    assert(bellman_ford(NULL, 0, dist, pred) == -1);
    GrafoListaPonderado *g = criar_grafo_ponderado(3);
    assert(g != NULL);
    assert(!inserir_aresta_ponderada(g, -1, 0, 1));
    assert(!inserir_aresta_ponderada(g, 0, 3, 1));
    assert(bellman_ford(g, 3, dist, pred) == -1);
    conferir_falha(3, dist, pred);
    assert(bellman_ford(g, -1, dist, pred) == -1);
    assert(bellman_ford(g, 0, NULL, pred) == -1);
    assert(bellman_ford(g, 0, dist, NULL) == -1);
    assert(bellman_ford(g, 0, dist, dist) == -1);
    assert(bellman_ford_com_testemunha(g, 0, dist, pred, NULL) == -1);
    assert(bellman_ford_com_testemunha(g, 4, dist, pred, &testemunha) == -1);
    assert(testemunha == -1);
    assert(inserir_aresta_ponderada(g, 0, 1, 0));
    g->adj[0]->destino = 3;
    assert(bellman_ford(g, 0, dist, pred) == -1);
    conferir_falha(3, dist, pred);
    liberar_grafo_ponderado(g);
    liberar_grafo_ponderado(NULL);

    FILE *saida = tmpfile();
    assert(saida != NULL);
    int ruim[] = {1, -1, 0};
    assert(reconstruir_ciclo_negativo(3, ruim, 0, saida) == -1);
    ruim[1] = 3;
    assert(reconstruir_ciclo_negativo(3, ruim, 0, saida) == -1);
    assert(reconstruir_ciclo_negativo(3, ruim, 3, saida) == -1);
    assert(reconstruir_ciclo_negativo(3, ruim, -1, saida) == -1);
    assert(reconstruir_ciclo_negativo(3, NULL, 0, saida) == -1);
    assert(reconstruir_ciclo_negativo(0, ruim, 0, saida) == -1);
    assert(reconstruir_ciclo_negativo(3, ruim, 0, NULL) == -1);
    assert(ftell(saida) == 0);
    assert(fclose(saida) == 0);
}

int main(void) {
    testar_cenarios();
    testar_casos_limite();
    testar_invalidos();
    puts("Todos os testes de Bellman-Ford passaram.");
    return 0;
}
