#ifndef COBERTURA_H
#define COBERTURA_H

typedef struct { int u, v; } Aresta;
/* Vetor privado de arestas logicas, uma vez por aresta nao orientada,
 * como o vetor de arestas da pratica08. Nenhum algoritmo altera o grafo. */
typedef struct Grafo Grafo;

typedef enum {
    COBERTURA_SUCESSO = 0,
    COBERTURA_ENTRADA_INVALIDA,
    COBERTURA_SEM_MEMORIA
} StatusCobertura;

typedef struct {
    int *vertices;             /* conjunto de indices distintos */
    int quantidade;
    StatusCobertura status;
} ResultadoCobertura;

/* Copia a entrada; n,m >= 0; m>0 exige vetor de m arestas. Extremos em
 * [0,n). Rejeita lacos e duplicatas, inclusive (u,v)/(v,u); nao normaliza.
 * n=0,m=0 e valido. NULL significa entrada invalida ou falta de memoria.
 * O chamador libera o grafo. Funcoes aceitam NULL ou grafo vivo desta API. */
Grafo *criar_grafo(int n, const Aresta *arestas, int m);
void liberar_grafo(Grafo *g);

/* 1: cobertura; 0: nao cobertura; -1: argumento invalido. quantidade em
 * [0,n], indices distintos em [0,n); quantidade>0 exige vetor suficiente.
 * NULL,0 representa conjunto vazio. Percorre todas as arestas para decidir.
 * Sem arestas, qualquer conjunto valido (inclusive vazio) e cobertura. */
int eh_cobertura(const Grafo *g, const int *vertices, int quantidade);

/* Resultado por valor: sucesso vazio = {NULL,0,COBERTURA_SUCESSO}; falhas
 * zeram ponteiro/cardinalidade e distinguem entrada invalida de alocacao.
 * Resultado possui vetor alocado, liberado por liberar_resultado_cobertura;
 * liberar antes de sobrescrever. Liberar falhas/vazio e seguro. A ordem
 * dos vertices nao e parte do contrato. Nenhum estado global e utilizado.
 * BT: otimo global, poda cardinalidade >= melhor; pior caso exponencial,
 * O(2^n * (n+m)), memoria auxiliar O(n) incluindo pilha. Inicializacao
 * gulosa faz parte da chamada exata. Nao ha timeout nem limite de nos.
 * Gulosa: maior grau das arestas descobertas, empate pelo menor indice;
 * O(n*(n+m)), memoria O(n). Nao garante otimo nem fator de aproximacao 2. */
ResultadoCobertura cobertura_minima_bt(const Grafo *g);
ResultadoCobertura cobertura_gulosa(const Grafo *g);
void liberar_resultado_cobertura(ResultadoCobertura *r);

#endif
