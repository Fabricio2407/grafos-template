#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "agm.h"

static void exigir(int condicao, const char *expressao, int linha) {
    if (!condicao) {
        fprintf(stderr, "Teste falhou na linha %d: %s\n", linha, expressao);
        exit(EXIT_FAILURE);
    }
}

#define EXIGIR(expr) exigir((expr), #expr, __LINE__)

/* Verificador independente: conjuntos por rotulos, sem usar nosso DSU. */
static void juntar_rotulos(int *rotulo, int n, int u, int v) {
    int antigo = rotulo[v];
    int novo = rotulo[u];
    for (int i = 0; i < n; i++) {
        if (rotulo[i] == antigo) {
            rotulo[i] = novo;
        }
    }
}

static void validar_resultado(int n, const Aresta *entrada, int m,
                             const ResultadoAGM *r) {
    /* Os casos de verificacao sao pequenos, com ate oito vertices. */
    EXIGIR(n >= 0 && n <= 8);
    EXIGIR(r->status == AGM_SUCESSO);
    EXIGIR(r->quantidade >= 0 && r->componentes >= 0 && r->componentes <= n);
    EXIGIR(r->quantidade == n - r->componentes);
    EXIGIR(r->quantidade == 0 || r->arestas != NULL);
    int original[8], floresta[8];
    int conectado[8][8] = {{0}};
    int maior_peso[8][8];
    for (int i = 0; i < n; i++) {
        original[i] = floresta[i] = i;
        conectado[i][i] = 1;
        for (int j = 0; j < n; j++) {
            maior_peso[i][j] = INT_MIN;
        }
    }
    for (int i = 0; i < m; i++) {
        juntar_rotulos(original, n, entrada[i].u, entrada[i].v);
    }
    int64_t soma = 0;
    for (int i = 0; i < r->quantidade; i++) {
        Aresta a = r->arestas[i];
        EXIGIR(a.u >= 0 && a.u < n && a.v >= 0 && a.v < n);
        EXIGIR(floresta[a.u] != floresta[a.v]); /* Rejeita ciclos e lacos. */
        int existe = 0;
        for (int j = 0; j < m; j++) {
            Aresta b = entrada[j];
            if (a.peso == b.peso && ((a.u == b.u && a.v == b.v) ||
                                     (a.u == b.v && a.v == b.u))) {
                existe = 1;
            }
        }
        EXIGIR(existe);
        juntar_rotulos(floresta, n, a.u, a.v);
        conectado[a.u][a.v] = conectado[a.v][a.u] = 1;
        maior_peso[a.u][a.v] = maior_peso[a.v][a.u] = a.peso;
        soma += a.peso;
    }
    EXIGIR(soma == r->custo_total);
    int componentes = 0;
    for (int i = 0; i < n; i++) {
        if (original[i] == i) {
            componentes++;
        }
        for (int j = 0; j < n; j++) {
            EXIGIR((original[i] == original[j]) == (floresta[i] == floresta[j]));
        }
    }
    EXIGIR(componentes == r->componentes);

    /* Alem da estrutura, verifica otimalidade pela propriedade de troca:
     * nenhuma aresta da entrada melhora o maior peso no caminho da floresta.
     * Como a saida ja foi validada como floresta, cada caminho e unico. */
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (!conectado[i][j] && conectado[i][k] && conectado[k][j]) {
                    conectado[i][j] = 1;
                    maior_peso[i][j] = maior_peso[i][k] > maior_peso[k][j]
                                          ? maior_peso[i][k] : maior_peso[k][j];
                }
            }
        }
    }
    for (int i = 0; i < m; i++) {
        Aresta a = entrada[i];
        if (a.u != a.v) {
            EXIGIR(conectado[a.u][a.v]);
            EXIGIR(maior_peso[a.u][a.v] <= a.peso);
        }
    }
}

static void testar_grafo(int n, const Aresta *arestas, int m,
                         int componentes, int64_t custo) {
    Aresta copia[12];
    EXIGIR(m >= 0 && m <= 12);
    if (m > 0) {
        memcpy(copia, arestas, (size_t)m * sizeof(*arestas));
    }
    GrafoListaPonderado *g = criar_grafo_ponderado(n, arestas, m);
    EXIGIR(g != NULL);
    ResultadoAGM k = kruskal(g);
    ResultadoAGM p = prim(g);
    validar_resultado(n, arestas, m, &k);
    validar_resultado(n, arestas, m, &p);
    EXIGIR(k.custo_total == p.custo_total && k.componentes == p.componentes);
    if (componentes >= 0) {
        EXIGIR(k.componentes == componentes && k.custo_total == custo);
    }
    if (m > 0) {
        EXIGIR(memcmp(copia, arestas, (size_t)m * sizeof(*arestas)) == 0);
    }
    liberar_resultado_agm(&k);
    liberar_resultado_agm(&p);
    /* Repetir apos ambos os algoritmos detecta alteracoes persistentes. */
    k = kruskal(g);
    validar_resultado(n, arestas, m, &k);
    liberar_resultado_agm(&k);
    liberar_resultado_agm(&k);
    liberar_grafo_ponderado(g);
}

static void conferir_heap(MinHeap *h) {
    int presentes = 0;
    for (int v = 0; v < h->capacidade; v++) {
        int i = h->posicao[v];
        if (i != -1) {
            EXIGIR(i >= 0 && i < h->tamanho);
            EXIGIR(h->dados[i].vertice == v);
            presentes++;
        }
    }
    EXIGIR(presentes == h->tamanho);
    for (int i = 0; i < h->tamanho; i++) {
        EXIGIR(h->posicao[h->dados[i].vertice] == i);
        if (i > 0) {
            EXIGIR(h->dados[(i - 1) / 2].chave <= h->dados[i].chave);
        }
    }
}

static void testar_estruturas(void) {
    DSU *d = criar_dsu(8);
    EXIGIR(d != NULL);
    for (int i = 0; i < 8; i++) {
        EXIGIR(d->pai[i] == i && d->rank[i] == 0);
    }
    EXIGIR(unir(d, 0, 1) == 1 && unir(d, 2, 3) == 1);
    EXIGIR(unir(d, 0, 2) == 1 && unir(d, 4, 5) == 1);
    EXIGIR(unir(d, 6, 7) == 1 && unir(d, 4, 6) == 1);
    EXIGIR(unir(d, 0, 4) == 1);
    EXIGIR(d->rank[0] == 3 && find(d, 7) == 0);
    EXIGIR(d->pai[7] == 0 && d->pai[6] == 0);
    EXIGIR(unir(d, 7, 3) == 0 && d->rank[0] == 3);
    EXIGIR(unir(d, -1, 0) == -1 && unir(d, 0, 8) == -1);
    EXIGIR(find(d, -1) == -1 && find(NULL, 0) == -1);
    liberar_dsu(d);
    d = criar_dsu(3);
    EXIGIR(d != NULL);
    EXIGIR(unir(d, 0, 1) == 1);
    EXIGIR(unir(d, 2, 0) == 1 && d->rank[0] == 1);
    liberar_dsu(d);
    EXIGIR(criar_dsu(-1) == NULL);
    d = criar_dsu(0);
    EXIGIR(d != NULL && find(d, 0) == -1);
    liberar_dsu(d);
    liberar_dsu(NULL);

    MinHeap *h = criar_min_heap(11);
    EXIGIR(h != NULL && criar_min_heap(0) == NULL);
    NoHeap minimo = {-7, -7};
    EXIGIR(!extrair_minimo(h, &minimo) && minimo.vertice == -7);
    EXIGIR(!inserir_min_heap(h, -1, 0) && !inserir_min_heap(h, 11, 0));
    for (int i = 0; i < 11; i++) {
        EXIGIR(inserir_min_heap(h, i, INT64_MAX - i));
        conferir_heap(h);
    }
    EXIGIR(!inserir_min_heap(h, 0, 1));
    EXIGIR(!diminuir_chave(h, 0, INT64_MAX));
    EXIGIR(diminuir_chave(h, 0, 1));
    conferir_heap(h);
    EXIGIR(diminuir_chave(h, 0, -1));
    conferir_heap(h);
    EXIGIR(diminuir_chave(h, 0, INT64_MIN));
    conferir_heap(h);
    EXIGIR(!diminuir_chave(h, 0, 0));
    EXIGIR(!diminuir_chave(h, 11, 0));
    EXIGIR(!extrair_minimo(h, NULL));
    int64_t anterior = INT64_MIN;
    while (extrair_minimo(h, &minimo)) {
        EXIGIR(minimo.chave >= anterior);
        anterior = minimo.chave;
        EXIGIR(h->posicao[minimo.vertice] == -1);
        EXIGIR(!diminuir_chave(h, minimo.vertice, INT64_MIN));
        conferir_heap(h);
    }
    EXIGIR(inserir_min_heap(h, 0, 0));
    EXIGIR(!inserir_min_heap(h, 0, -1));
    conferir_heap(h);
    liberar_min_heap(h);
    liberar_min_heap(NULL);
}

static void testar_casos_adicionais(void) {
    testar_estruturas();
    testar_grafo(0, NULL, 0, 0, 0);
    testar_grafo(1, NULL, 0, 1, 0);
    testar_grafo(5, NULL, 0, 5, 0);
    const Aresta especiais[] = {{0, 0, INT_MIN}, {0, 1, 5}, {1, 0, -2},
                                {1, 2, 0}, {0, 2, 10}, {2, 2, -9}};
    testar_grafo(3, especiais, 6, 1, -2);
    const Aresta grandes[] = {{0, 1, INT_MAX}, {1, 2, INT_MAX}};
    testar_grafo(3, grandes, 2, 1, (int64_t)INT_MAX * 2);
    const Aresta pequenos[] = {{0, 1, INT_MIN}, {1, 2, INT_MIN}, {0, 2, INT_MAX}};
    testar_grafo(3, pequenos, 3, 1, (int64_t)INT_MIN * 2);
    const Aresta lacos[] = {{0, 0, -1}, {1, 1, 0}, {2, 2, INT_MAX}};
    testar_grafo(3, lacos, 3, 3, 0);
    const Aresta zero[] = {{0, 1, 0}, {1, 2, 0}, {2, 0, 0}};
    testar_grafo(3, zero, 3, 1, 0);
    const Aresta laco_unico[] = {{0, 0, INT_MIN}};
    testar_grafo(1, laco_unico, 1, 1, 0);
    EXIGIR(criar_grafo_ponderado(-1, NULL, 0) == NULL);
    EXIGIR(criar_grafo_ponderado(1, NULL, -1) == NULL);
    EXIGIR(criar_grafo_ponderado(1, NULL, 1) == NULL);
    const Aresta invalida[] = {{-1, 0, 0}, {0, 2, 0}};
    EXIGIR(criar_grafo_ponderado(2, invalida, 1) == NULL);
    EXIGIR(criar_grafo_ponderado(2, invalida + 1, 1) == NULL);
    EXIGIR(criar_grafo_ponderado(0, laco_unico, 1) == NULL);
    ResultadoAGM r = kruskal(NULL);
    EXIGIR(r.status == AGM_ENTRADA_INVALIDA && r.arestas == NULL);
    EXIGIR(r.quantidade == 0 && r.custo_total == 0 && r.componentes == 0);
    liberar_resultado_agm(&r);
    r = prim(NULL);
    EXIGIR(r.status == AGM_ENTRADA_INVALIDA && r.arestas == NULL);
    EXIGIR(r.quantidade == 0 && r.custo_total == 0 && r.componentes == 0);
    liberar_resultado_agm(&r);
    liberar_resultado_agm(NULL);
    liberar_grafo_ponderado(NULL);

    /* Casos deterministas adicionais, sem depender de rand() ou do relogio. */
    uint32_t estado = 12345;
    for (int caso = 0; caso < 80; caso++) {
        int n = 1 + caso % 7;
        int m = caso % 11;
        Aresta a[10];
        for (int i = 0; i < m; i++) {
            estado = estado * UINT32_C(1664525) + UINT32_C(1013904223);
            a[i].u = (int)(estado % (uint32_t)n);
            estado = estado * UINT32_C(1664525) + UINT32_C(1013904223);
            a[i].v = (int)(estado % (uint32_t)n);
            estado = estado * UINT32_C(1664525) + UINT32_C(1013904223);
            a[i].peso = (int)(estado % 17) - 8;
            if (caso % 9 == 0) {
                a[i].peso = i % 2 == 0 ? INT_MIN : INT_MAX;
            }
        }
        testar_grafo(n, a, m, -1, 0);
    }
}

static void mostrar_resultado(const char *nome, const ResultadoAGM *r) {
    printf("%s:\n", nome);
    for (int i = 0; i < r->quantidade; i++) {
        Aresta a = r->arestas[i];
        printf("  %d -- %d, peso %d\n", a.u, a.v, a.peso);
    }
    printf("Arestas: %d | Custo total: %" PRId64 " | Componentes: %d\n",
           r->quantidade, r->custo_total, r->componentes);
    if (r->componentes > 1) {
        printf("Floresta geradora minima: grafo desconexo; nao existe uma AGM "
               "abrangendo todos os vertices.\n");
    }
}

static int demonstrar(const char *nome, int n, const Aresta *arestas, int m) {
    GrafoListaPonderado *g = criar_grafo_ponderado(n, arestas, m);
    if (g == NULL) {
        fprintf(stderr, "Falha ao criar grafo.\n");
        return 0;
    }
    ResultadoAGM k = kruskal(g);
    ResultadoAGM p = prim(g);
    int sucesso = k.status == AGM_SUCESSO && p.status == AGM_SUCESSO;
    if (sucesso) {
        printf("\n=== %s ===\n", nome);
        mostrar_resultado("Kruskal", &k);
        mostrar_resultado("Prim", &p);
    } else {
        fprintf(stderr, "Falha nos algoritmos: Kruskal=%d, Prim=%d.\n",
                (int)k.status, (int)p.status);
    }
    liberar_resultado_agm(&k);
    liberar_resultado_agm(&p);
    liberar_grafo_ponderado(g);
    return sucesso;
}

int main(int argc, char **argv) {
    const Aresta a[] = {{0, 1, 2}, {0, 2, 3}, {1, 2, 1}, {1, 3, 4},
                        {2, 3, 5}, {2, 4, 7}, {3, 4, 6}};
    const Aresta b[] = {{0, 1, 2}, {1, 2, 1}, {0, 2, 4}, {3, 4, -2}};
    const Aresta c[] = {{0, 1, 1}, {1, 2, 1}, {2, 3, 1}, {3, 0, 1}, {0, 2, 2}};
    if (argc == 2 && strcmp(argv[1], "--test") == 0) {
        testar_grafo(5, a, 7, 1, 13);
        testar_grafo(6, b, 4, 3, 1);
        testar_grafo(4, c, 5, 1, 3);
        testar_casos_adicionais();
        puts("Todos os testes de AGM passaram (incluindo 80 grafos adicionais).");
        return EXIT_SUCCESS;
    }
    if (argc != 1) {
        fprintf(stderr, "Uso: %s [--test]\n", argv[0]);
        return EXIT_FAILURE;
    }
    int sucesso = demonstrar("A: grafo conexo", 5, a, 7);
    sucesso &= demonstrar("B: grafo desconexo", 6, b, 4);
    sucesso &= demonstrar("C: pesos repetidos", 4, c, 5);
    return sucesso ? EXIT_SUCCESS : EXIT_FAILURE;
}
