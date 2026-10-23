#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <limits.h>

typedef struct {
    int vertice;
    int distancia;
} NoHeap;

typedef struct {
    NoHeap *dados;
    int capacidade, tamanho;
    int *posicao;
} MinHeap;

typedef struct NoPonderado {
    int destino, peso;
    struct NoPonderado *prox;
} NoPonderado;

typedef struct {
    int n;
    NoPonderado **adj;
} GrafoListaPonderado;

/* Capacidade positiva; vertices em [0, capacidade). NULL indica falha. */
MinHeap *criar_min_heap(int capacidade);
/* Retornam 1 no sucesso, 0 para argumentos invalidos ou operacao recusada.
 * Insercao recusa duplicatas e heap cheio. Chave igual/maior e recusada.
 * Extracao vazia nao altera *minimo. posicao vale -1 para ausentes. */
int inserir_min_heap(MinHeap *heap, int vertice, int distancia);
int extrair_minimo(MinHeap *heap, NoHeap *minimo);
int diminuir_chave(MinHeap *heap, int vertice, int distancia);
void liberar_min_heap(MinHeap *heap);

/* Digrafo: criacao exige n > 0. Insercao retorna 1 ou 0 e rejeita pesos
 * negativos com diagnostico. Arestas paralelas, lacos e peso zero sao aceitos. */
GrafoListaPonderado *criar_grafo_ponderado(int n);
int inserir_aresta_ponderada(GrafoListaPonderado *g, int u, int v, int peso);
void liberar_grafo_ponderado(GrafoListaPonderado *g);

/* dist e pred devem apontar para vetores distintos com pelo menos g->n ints.
 * INT_MAX significa infinito; custos representaveis sao < INT_MAX.
 * Argumentos invalidos, peso negativo (mesmo inalcançavel), alocacao ou
 * soma >= INT_MAX abortam com mensagem em stderr, sem resultados parciais:
 * quando g->n > 0, todos os vetores fornecidos sao reinicializados para
 * dist = INT_MAX e pred = -1. Sem grafo valido nao ha tamanho para limpa-los.
 * Sucesso pode ser verificado por dist[origem] == 0.
 * O((V + E) log V), memoria auxiliar O(V), alem do grafo.
 * O grafo deve possuir listas validas, aciclicas e destinos em [0, n).
 */
void dijkstra(GrafoListaPonderado *g, int origem, int *dist, int *pred);

/* Indices validos; pred deve vir de Dijkstra bem-sucedido no mesmo grafo
 * e origem. Sem o tamanho no contrato, esta funcao nao valida limites nem
 * vetores arbitrarios. Reconstrucao iterativa, O(V) de tempo e memoria. */
void reconstruir_caminho(int origem, int destino, int *pred);

#endif
