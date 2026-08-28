#include <stdio.h>

#include "grafo_lista.h"
#include "grafo_matriz.h"

int main(void) {
    int numero_vertices = 8;
    GrafoMatriz *matriz = criar_grafo_matriz(numero_vertices);
    GrafoLista *lista = criar_grafo_lista(numero_vertices);
    if (matriz == NULL || lista == NULL) {
        liberar_grafo_matriz(matriz);
        liberar_grafo_lista(lista);
        return 1;
    }

        inserir_aresta_matriz(matriz, 0, 1);
        inserir_aresta_matriz(matriz, 0, 2);
        inserir_aresta_matriz(matriz, 0, 3);
        inserir_aresta_matriz(matriz, 1, 4);
        inserir_aresta_matriz(matriz, 1, 5);
        inserir_aresta_matriz(matriz, 2, 3);
        inserir_aresta_matriz(matriz, 2, 6);
        inserir_aresta_matriz(matriz, 3, 6);
        inserir_aresta_matriz(matriz, 4, 5);
        inserir_aresta_matriz(matriz, 7, 4);
        inserir_aresta_matriz(matriz, 7, 5);
        inserir_aresta_matriz(matriz, 7, 6);

        inserir_aresta_lista(lista, 0, 1);
        inserir_aresta_lista(lista, 0, 2);
        inserir_aresta_lista(lista, 0, 3);
        inserir_aresta_lista(lista, 1, 4);
        inserir_aresta_lista(lista, 1, 5);
        inserir_aresta_lista(lista, 2, 3);
        inserir_aresta_lista(lista, 2, 6);
        inserir_aresta_lista(lista, 3, 6);
        inserir_aresta_lista(lista, 4, 5);
        inserir_aresta_lista(lista, 7, 4);
        inserir_aresta_lista(lista, 7, 5);
        inserir_aresta_lista(lista, 7, 6);

        printf("Matriz de Adjacencia\n");
        exibir_matriz(matriz);
        printf("\nLista de Adjacencia\n");
        exibir_lista(lista);
        printf("\nGrau do vertice 0: matriz = %d, lista = %d\n",
            grau_matriz(matriz, 0), grau_lista(lista, 0));
        printf("Vertices 0 e 1 adjacentes: matriz = %d, lista = %d\n",
            sao_adjacentes_matriz(matriz, 0, 1), sao_adjacentes_lista(lista, 0, 1));

        remover_aresta_matriz(matriz, 0, 1);
        remover_aresta_lista(lista, 0, 1);
        printf("\nAresta 0-1 removida: matriz = %d, lista = %d\n",
            sao_adjacentes_matriz(matriz, 0, 1), sao_adjacentes_lista(lista, 0, 1));

    liberar_grafo_matriz(matriz);
    liberar_grafo_lista(lista);
    return 0;
}
