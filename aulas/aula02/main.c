#include <stdio.h>
#include "grafo_matriz.h"

int main() {
    int numero_vertices = 8;
    GrafoMatriz grafo;

    inicializar(&grafo, numero_vertices);

    inserir_aresta(&grafo, 0, 1);
    inserir_aresta(&grafo, 0, 2);
    inserir_aresta(&grafo, 0, 3);
    inserir_aresta(&grafo, 1, 4);
    inserir_aresta(&grafo, 1, 5);
    inserir_aresta(&grafo, 2, 3);
    inserir_aresta(&grafo, 2, 6);
    inserir_aresta(&grafo, 3, 6);
    inserir_aresta(&grafo, 4, 5);
    inserir_aresta(&grafo, 7, 4);
    inserir_aresta(&grafo, 7, 5);
    inserir_aresta(&grafo, 7, 6);

    printf("Matriz de Adjacencia\n");
    exibir_matriz(&grafo);

    inicializar(&grafo, 3);
    inserir_arco(&grafo, 0, 1);
    inserir_arco(&grafo, 1, 2);
    inserir_arco(&grafo, 2, 0);

    printf("Matriz de Adjacencia (Arcos)\n");
    exibir_matriz(&grafo);

    return 0;
}

    // 1 -- 2
    // 1 -- 3
    // 1 -- 4
    // 2 -- 5
    // 2 -- 6
    // 3 -- 4
    // 3 -- 7
    // 4 -- 7
    // 5 -- 6
    // 8 -- 5
    // 8 -- 6
    // 8 -- 7