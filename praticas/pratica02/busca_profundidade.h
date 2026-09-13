#ifndef BUSCA_PROFUNDIDADE_H
#define BUSCA_PROFUNDIDADE_H

#include "busca_largura.h"

typedef struct {
    int *dados;
    int topo, capacidade;
} Pilha;

void dfs_recursiva(GrafoLista *g, int u, int *visitado,
                   int *entrada, int *saida, int *tempo);
int contar_componentes(GrafoLista *g);
int tem_ciclo(GrafoLista *g);

#endif
