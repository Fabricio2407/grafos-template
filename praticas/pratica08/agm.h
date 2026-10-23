#ifndef AGM_H
#define AGM_H

#include <stdint.h>

typedef struct {
    int *pai, *rank;
    int n;
} DSU;

typedef struct {
    int u, v, peso;
} Aresta;

typedef struct NoPonderado {
    int destino, peso;
    struct NoPonderado *prox;
} NoPonderado;

/* Grafo nao orientado com listas de adjacencia. Definicao privada em agm.c
 * impede alterar a simetria das listas. Cada aresta logica e copiada uma vez
 * no vetor interno e duas vezes nas listas, inclusive lacos. */
typedef struct GrafoListaPonderado GrafoListaPonderado;

typedef struct {
    int vertice;
    int64_t chave;
} NoHeap;

typedef struct {
    NoHeap *dados;
    int capacidade, tamanho;
    int *posicao;
} MinHeap;

typedef enum {
    AGM_SUCESSO = 0,
    AGM_ENTRADA_INVALIDA,
    AGM_SEM_MEMORIA,
    AGM_OVERFLOW
} StatusAGM;

typedef struct {
    Aresta *arestas;
    int quantidade;
    int64_t custo_total;
    int componentes;
    StatusAGM status;
} ResultadoAGM;

/* n >= 0; NULL em falha/argumento invalido. DSU so deve ser modificado pelas
 * funcoes abaixo. find retorna raiz ou -1 para indice/ponteiro invalido.
 * unir retorna 1 se uniu, 0 se redundante, -1 se argumento invalido. */
DSU *criar_dsu(int n);
int find(DSU *d, int x);
int unir(DSU *d, int x, int y);
void liberar_dsu(DSU *d);

/* Capacidade > 0; vertices em [0,capacidade). NULL em erro de criacao.
 * Operacoes retornam 1 no sucesso, 0 se recusadas: indice invalido,
 * duplicata, cheio/vazio, chave igual/maior ou vertice ausente.
 * Extracao recusada nao altera *minimo. posicao=-1 para ausentes/extraidos.
 * Cada operacao e O(log V), sem busca linear. Nao alterar os campos. */
MinHeap *criar_min_heap(int capacidade);
int inserir_min_heap(MinHeap *h, int vertice, int64_t chave);
int diminuir_chave(MinHeap *h, int vertice, int64_t chave);
int extrair_minimo(MinHeap *h, NoHeap *minimo);
void liberar_min_heap(MinHeap *h);

/* Copia arestas: n,m >= 0; m>0 exige vetor com m elementos e extremos
 * em [0,n). Aceita lacos, paralelas e TODOS os pesos int.
 * Retorna NULL em entrada invalida/alocacao; nao altera a entrada.
 * Grafo vazio n=0,m=0 e aceito. A copia pertence ao grafo. */
GrafoListaPonderado *criar_grafo_ponderado(int n, const Aresta *arestas, int m);
void liberar_grafo_ponderado(GrafoListaPonderado *g);

/* g deve ser NULL ou grafo vivo criado pela funcao acima.
 * Ambos retornam floresta minima; componentes>1 indica grafo desconexo,
 * sem AGM abrangendo todos os vertices. Vazio: sucesso, custo/arestas/
 * componentes=0. Vertice isolado conta como componente. Lacos ignorados.
 * O resultado possui arestas alocadas; liberar mesmo apos falha e seguro.
 * Em falha, status distingue causa e todos os demais campos ficam zerados,
 * sem solucao parcial. Resultado deve ser liberado antes de ser sobrescrito.
 * Soma em int64_t com checagem; peso INT_MAX e valido.
 * Kruskal: O(V + E log(E+1)), heapsort sobre copia, memoria O(V+E).
 * Prim: O((V+E) log V) para V>1; V<=1 tratado diretamente. Memoria O(V).
 * Nenhum algoritmo modifica o grafo ou o vetor original.
 */
ResultadoAGM kruskal(const GrafoListaPonderado *g);
ResultadoAGM prim(const GrafoListaPonderado *g);
void liberar_resultado_agm(ResultadoAGM *r);

#endif
