#include "grafo_lista.h"

#ifndef BUSCA_GRAFO
#define BUSCA_GRAFO

void dfs(GrafoLista *g, int u, int *p, int *visitado);
void bfs(GrafoLista *g, int u);

#endif