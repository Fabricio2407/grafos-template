#ifndef CONECTIVIDADE_H
#define CONECTIVIDADE_H

typedef struct No {
    int destino;
    struct No *prox;
} No;

typedef struct {
    int n;
    No **adj;
} GrafoLista;

typedef struct {
    int u;
    int v;
} Ponte;

GrafoLista *criar_grafo_lista(int n);
void inserir_aresta_lista(GrafoLista *g, int u, int v);
int contar_arestas(GrafoLista *g);
void liberar_grafo_lista(GrafoLista *g);

void dfs_articulacoes(GrafoLista *g, int u, int *visitado,
                      int *descoberta, int *low, int *pai,
                      int *articulacao, int *tempo);
int *encontrar_articulacoes(GrafoLista *g);
Ponte *detectar_pontes(GrafoLista *g, int *quantidade);

#endif
