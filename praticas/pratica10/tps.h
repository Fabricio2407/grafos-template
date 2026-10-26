#ifndef TPS_H
#define TPS_H

#include "hamiltoniano.h"

/* Completo nao orientado: matriz n*n simetrica, diagonal zero, pesos
 * int64_t >=0. Zero e peso valido, nunca ausencia. Pode ser inicializado
 * com matriz do chamador; algoritmos apenas emprestam/leem essa memoria. */
typedef struct { int n; int64_t *pesos; } GrafoTSP;
typedef struct {
    Status status;
    int *vertices;
    int quantidade;
    int64_t custo;
    uint64_t rotas_avaliadas; /* somente forca bruta; heuristicas: zero */
} ResultadoTSP;

/* Resultados possuem vetor: liberar antes de sobrescrever. Em erro/vazio
 * vetor NULL, quantidade/custo zero (contador pode registrar enumeracao).
 * n=0: SEM_SOLUCAO; n=1: [0,0], custo 0, uma permutacao vazia;
 * n=2: [0,1,0], custo 2*peso(0,1). Estas sao rotas, nao ciclos simples.
 * Somatorios checados: OVERFLOW se custo produzido nao cabe em int64_t.
 * Exata continua enumerando rotas que excedem int64_t, pois uma outra
 * pode caber; retorna OVERFLOW apenas se nenhuma cabe ou se (n-1)!
 * nao cabe no contador uint64_t (verificado antes da busca).
 * Entrada nunca e alterada. Memoria auxiliar O(n), alem da matriz. */
ResultadoTSP tsp_forca_bruta(const GrafoTSP *g);
ResultadoTSP tsp_vizinho_mais_proximo(const GrafoTSP *g);
ResultadoTSP tsp_insercao(const GrafoTSP *g);
void liberar_resultado_tsp(ResultadoTSP *r);

/* Exata fixa 0 e enumera TODAS as (n-1)! permutacoes, incrementalmente,
 * sem branch-and-bound. Avaliar cada rota do zero custaria O(n), total
 * O(n*(n-1)!). Acumular custos evita recomputar todas as suas arestas;
 * esta implementacao ainda percorre n candidatos em cada no interno.
 * Vizinho: O(n^2), empate menor indice. Insercao: O(n^3), primeiro
 * escolhe vertice de menor distancia a rota (empate menor indice), depois
 * menor variacao (empate primeira aresta na ordem atual da rota).
 * Ambas heuristicas podem ser subotimas. Sem hipotese de desigualdade
 * triangular, nao se promete fator de aproximacao. */

/* Saida deve ser um descritor disponivel (sem matriz a perder).
 * Sucesso aloca matriz independente, inclusive grafo vazio {0,NULL};
 * chamador libera SOMENTE matrizes desta funcao com liberar_grafo_tsp.
 * Erro zera saida. Entrada/saida nao podem sobrepor memoria.
 * Pares com aresta recebem 1, sem aresta 2; diagonal 0.
 * Para n>=3: ciclo Hamiltoniano sse TSP otimo=n. Cada rota usa n pesos
 * >=1; soma n sse todos sao arestas originais. Otimo>n prova ausencia;
 * custo de heuristica>n nao prova. Reducao e de CICLO, nao de caminho. */
Status grafo_para_tsp(const Grafo *g, GrafoTSP *saida);
void liberar_grafo_tsp(GrafoTSP *g);

#endif
