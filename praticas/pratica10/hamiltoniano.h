#ifndef HAMILTONIANO_H
#define HAMILTONIANO_H

#include <stdint.h>

typedef enum {
    SOLUCAO = 0, SEM_SOLUCAO, ENTRADA_INVALIDA, SEM_MEMORIA, OVERFLOW
} Status;

/* Matrizes contiguas n*n, indices 0..n-1. O chamador fornece memoria
 * suficiente e conserva sua propriedade; os algoritmos nao a modificam.
 * n >= 0; n=0 admite NULL. Grafo simples: diagonal zero, simetria e 0/1.
 * Dimensoes que nao permitem representar/alocar a matriz sao invalidas. */
typedef struct { int n; const unsigned char *adj; } Grafo;
typedef struct {
    Status status;
    int *vertices;
    int quantidade;
} ResultadoHamiltoniano;

int grafo_valido(const Grafo *g);
/* Caminho vazio: SEM_SOLUCAO; n=1: caminho [0]. Ciclo simples exige n>=3.
 * Sucesso possui n vertices (caminho) ou n+1 (ciclo, origem repetida).
 * Sem solucao/erro: vetor NULL, quantidade zero. Chamador deve liberar
 * cada resultado antes de sobrescreve-lo; liberar NULL/resultado vazio
 * e seguro. Nao copiar resultados proprietarios para libera-los duas vezes.
 * BT tenta todas as origens para caminho; ciclo fixa 0. Pior caso fatorial
 * (limite conservador O(n*n!)); poda por adjacencia nao o elimina.
 * Memoria auxiliar O(n), incluindo pilha, alem da matriz O(n^2). */
ResultadoHamiltoniano encontrar_caminho_hamiltoniano_bt(const Grafo *g);
ResultadoHamiltoniano encontrar_ciclo_hamiltoniano_bt(const Grafo *g);
void liberar_resultado_hamiltoniano(ResultadoHamiltoniano *r);

#endif
