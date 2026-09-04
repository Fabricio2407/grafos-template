#include <stdio.h>
#include <string.h>
#include "grafo_lista.h"
#include "busca_grafo.h"

int main() {
    GrafoLista *g = criar_grafo(5);

    adicionar_aresta(g, 0, 1);
    adicionar_aresta(g, 0, 2);
    adicionar_aresta(g, 1, 3);
    adicionar_aresta(g, 2, 3);
    adicionar_aresta(g, 3, 4);

    for (int i = 0; i < g->num_vertices; i++) {
        printf("%i: -> ", i+1);
        No *no = g->lista[i];
        while (no != NULL) {
            printf("%i -> ", no->vertice + 1);
            no = no->proximo;
        }
        printf("NULL\n");
    }

    int pilha[10];
    int visitado[10];

    memset(visitado, 0, sizeof(visitado));
    printf("inicia dfs pelo vertice 1\n");
    dfs(g, 0, pilha, visitado);
    printf("\n");
    
    memset(visitado, 0, sizeof(visitado));
    printf("inicia dfs pelo vertice 2\n");
    dfs(g, 1, pilha, visitado);
    printf("\n");
    
    memset(visitado, 0, sizeof(visitado));
    printf("inicia dfs pelo vertice 3\n");
    dfs(g, 2, pilha, visitado);
    printf("\n");
    
    memset(visitado, 0, sizeof(visitado));
    printf("inicia dfs pelo vertice 4\n");
    dfs(g, 3, pilha, visitado);
    printf("\n");
    
    memset(visitado, 0, sizeof(visitado));
    printf("inicia dfs pelo vertice 5\n");
    dfs(g, 4, pilha, visitado);
    printf("\n");

    return 0;
}