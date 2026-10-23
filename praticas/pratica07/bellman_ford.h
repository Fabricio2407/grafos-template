#ifndef BELLMAN_FORD_H
#define BELLMAN_FORD_H

#include <limits.h>
#include <stdio.h>

/* Mesma representacao da pratica06, independente daquela implementacao. */
typedef struct NoPonderado {
    int destino, peso;
    struct NoPonderado *prox;
} NoPonderado;

typedef struct {
    int n;
    NoPonderado **adj;
} GrafoListaPonderado;

/* n > 0; NULL em argumento invalido ou falha de alocacao. */
GrafoListaPonderado *criar_grafo_ponderado(int n);
/* Digrafo: aceita qualquer peso int, lacos e arestas paralelas.
 * Retorna 1 no sucesso ou 0 em argumento invalido/falha de alocacao. */
int inserir_aresta_ponderada(GrafoListaPonderado *g, int u, int v, int peso);
void liberar_grafo_ponderado(GrafoListaPonderado *g);

/* dist/pred: vetores distintos de pelo menos g->n inteiros; origem em [0,n).
 * Listas devem ser aciclicas e possuir nos validos.
 * Retornos: 1 = caminhos minimos; 0 = ciclo negativo alcancavel;
 * -1 = argumento invalido, alocacao ou custo nao representavel (stderr).
 * Sucesso: dist em [INT_MIN,INT_MAX), INT_MAX para inalcancaveis; pred=-1
 * para origem/inalcancaveis. Calculo interno int64_t, sem saturacao.
 * Retorno 0: dist fica INT_MAX e pred serve SOMENTE para reconstruir ciclo,
 * nunca como solucao de caminhos minimos. Retorno -1: limpa dist/pred para
 * INT_MAX/-1 quando g->n > 0 e os vetores sao fornecidos. Sem tamanho valido
 * nao acessa os vetores. Buffers sobrepostos nao sao permitidos.
 * Tempo O(V*E) (mais inicializacao O(V)); memoria auxiliar O(V).
 */
int bellman_ford(GrafoListaPonderado *g, int origem, int *dist, int *pred);

/* Mesma execucao, mais *testemunha: ultimo vertice atualizado na n-esima
 * rodada no retorno 0; -1 nos demais retornos. Ponteiro obrigatorio, distinto
 * dos vetores. O wrapper acima usa esta funcao, sem estado global. */
int bellman_ford_com_testemunha(GrafoListaPonderado *g, int origem,
                              int *dist, int *pred, int *testemunha);

/* Usar apenas pred/testemunha do retorno 0 no mesmo grafo de n vertices.
 * Valida indices, retrocede exatamente n vezes e coleta ate n vertices.
 * Imprime em saida o ciclo fechado no sentido das arestas, seguido de '\n'.
 * Retorna 1 no sucesso ou -1 em dados invalidos, alocacao ou erro de escrita.
 * Nao verifica pesos: sem o grafo, nao pode atestar negatividade ou arestas.
 * O(V) de tempo/memoria. Nao imprime sequencia parcial por dados invalidos.
 */
int reconstruir_ciclo_negativo(int n, const int *pred, int testemunha,
                              FILE *saida);

#endif
