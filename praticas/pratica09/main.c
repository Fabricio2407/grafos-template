#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cobertura.h"

/* Verificacoes nao dependem de assert/NDEBUG e ficam fora dos tempos. */
static void exigir(int condicao, const char *mensagem) {
    if (!condicao) {
        fprintf(stderr, "Falha: %s\n", mensagem);
        exit(EXIT_FAILURE);
    }
}

/* Oraculo independente: enumera mascaras e consulta diretamente a entrada,
 * sem chamar eh_cobertura nem compartilhar a ramificacao do backtracking.
 * Restrito aos pequenos grafos de validacao (n<=15), nunca aos algoritmos. */
static int otimo_enumeracao(int n, const Aresta *a, int m) {
    exigir(n >= 0 && n <= 15, "limite do oraculo de teste");
    int melhor = n;
    uint32_t limite = UINT32_C(1) << n;
    for (uint32_t mascara = 0; mascara < limite; mascara++) {
        int quantidade = 0;
        for (int v = 0; v < n; v++) {
            quantidade += (mascara & (UINT32_C(1) << v)) != 0;
        }
        if (quantidade >= melhor) {
            continue;
        }
        int coberta = 1;
        for (int i = 0; i < m; i++) {
            if ((mascara & ((UINT32_C(1) << a[i].u) |
                            (UINT32_C(1) << a[i].v))) == 0) {
                coberta = 0;
                break;
            }
        }
        if (coberta) {
            melhor = quantidade;
        }
    }
    return melhor;
}

static void verificar_par(const Grafo *g, const ResultadoCobertura *bt,
                          const ResultadoCobertura *gulosa) {
    exigir(bt->status == COBERTURA_SUCESSO &&
           gulosa->status == COBERTURA_SUCESSO, "status dos algoritmos");
    exigir(eh_cobertura(g, bt->vertices, bt->quantidade) == 1,
           "validade da cobertura exata");
    exigir(eh_cobertura(g, gulosa->vertices, gulosa->quantidade) == 1,
           "validade da cobertura gulosa");
    exigir(bt->quantidade <= gulosa->quantidade, "exata <= gulosa");
}

static void mostrar(const char *nome, const ResultadoCobertura *r) {
    printf("  %s: {", nome);
    for (int i = 0; i < r->quantidade; i++) {
        printf("%s%d", i == 0 ? "" : ", ", r->vertices[i]);
    }
    printf("}, tamanho %d; cobertura valida\n", r->quantidade);
}

static void exemplo(const char *nome, int n, const Aresta *a, int m,
                    int esperado_bt, int esperado_gulosa) {
    Grafo *g = criar_grafo(n, a, m);
    exigir(g != NULL, "criacao do exemplo");
    ResultadoCobertura bt = cobertura_minima_bt(g);
    ResultadoCobertura gulosa = cobertura_gulosa(g);
    verificar_par(g, &bt, &gulosa);
    exigir(bt.quantidade == esperado_bt && gulosa.quantidade == esperado_gulosa,
           "cardinalidades esperadas dos exemplos");
    exigir(bt.quantidade == otimo_enumeracao(n, a, m), "oraculo dos exemplos");
    printf("\n%s\n", nome);
    mostrar("Exata", &bt);
    mostrar("Gulosa", &gulosa);
    liberar_resultado_cobertura(&bt);
    liberar_resultado_cobertura(&gulosa);
    liberar_grafo(g);
}

static void exemplos(void) {
    const Aresta caminho[] = {{0,1}, {1,2}, {2,3}, {3,4}};
    const Aresta triangulo[] = {{0,1}, {1,2}, {0,2}};
    const Aresta estrela[] = {{0,1}, {0,2}, {0,3}, {0,4}};
    const Aresta subotimo[] = {{0,1}, {0,2}, {0,3}, {1,4}, {2,5}, {3,6}};
    exemplo("A: sem arestas", 5, NULL, 0, 0, 0);
    exemplo("B: caminho", 5, caminho, 4, 2, 2);
    exemplo("C: triangulo", 3, triangulo, 3, 2, 2);
    exemplo("D: estrela", 5, estrela, 4, 1, 1);
    exemplo("E: gulosa subotima", 7, subotimo, 6, 3, 4);
}

static void testar_contratos(void) {
    const Aresta duplicada[] = {{0,1}, {1,0}};
    const Aresta repetida[] = {{0,1}, {0,1}};
    const Aresta laco[] = {{0,0}};
    const Aresta invalida[] = {{0,2}};
    const Aresta negativa[] = {{-1,1}};
    exigir(criar_grafo(-1, NULL, 0) == NULL, "n negativo");
    exigir(criar_grafo(2, NULL, -1) == NULL, "m negativo");
    exigir(criar_grafo(2, NULL, 1) == NULL, "arestas nulas");
    exigir(criar_grafo(2, duplicada, 2) == NULL, "duplicata inversa");
    exigir(criar_grafo(2, repetida, 2) == NULL, "duplicata direta");
    exigir(criar_grafo(2, laco, 1) == NULL, "laco");
    exigir(criar_grafo(2, invalida, 1) == NULL, "extremo fora do intervalo");
    exigir(criar_grafo(2, negativa, 1) == NULL, "extremo negativo");
    exigir(criar_grafo(0, laco, 1) == NULL, "aresta em grafo sem vertices");
    ResultadoCobertura b = cobertura_minima_bt(NULL), h = cobertura_gulosa(NULL);
    exigir(b.status == COBERTURA_ENTRADA_INVALIDA && b.vertices == NULL &&
           b.quantidade == 0 && h.status == COBERTURA_ENTRADA_INVALIDA &&
           h.vertices == NULL && h.quantidade == 0, "algoritmos com NULL");
    liberar_resultado_cobertura(&b);
    liberar_resultado_cobertura(&h);
    Grafo *vazio = criar_grafo(0, NULL, 0);
    exigir(vazio != NULL, "grafo com zero vertices");
    b = cobertura_minima_bt(vazio);
    h = cobertura_gulosa(vazio);
    verificar_par(vazio, &b, &h);
    exigir(b.quantidade == 0 && h.quantidade == 0 &&
           b.vertices == NULL && h.vertices == NULL, "sucesso vazio");
    liberar_resultado_cobertura(&b);
    liberar_resultado_cobertura(&b);
    liberar_resultado_cobertura(&h);
    liberar_grafo(vazio);
    Aresta entrada[] = {{0,1}, {1,2}};
    Grafo *g = criar_grafo(4, entrada, 2);
    exigir(g != NULL, "grafo para contratos");
    entrada[0] = (Aresta){2,3}; /* grafo deve possuir copia independente */
    const int centro[] = {1}, incompleta[] = {0}, fora[] = {4};
    const int neg[] = {-1}, repetidos[] = {1,1}, todos[] = {0,1,2,3};
    exigir(eh_cobertura(g, centro, 1) == 1, "copia da entrada e cobertura");
    exigir(eh_cobertura(g, incompleta, 1) == 0, "cobertura incompleta");
    exigir(eh_cobertura(g, NULL, 0) == 0, "vazio nao cobre arestas");
    exigir(eh_cobertura(NULL, NULL, 0) == -1, "grafo nulo");
    exigir(eh_cobertura(g, NULL, 1) == -1, "conjunto nulo");
    exigir(eh_cobertura(g, centro, -1) == -1, "cardinalidade negativa");
    exigir(eh_cobertura(g, todos, 5) == -1, "cardinalidade acima de n");
    exigir(eh_cobertura(g, fora, 1) == -1, "indice fora");
    exigir(eh_cobertura(g, neg, 1) == -1, "indice negativo");
    exigir(eh_cobertura(g, repetidos, 2) == -1, "conjunto repetido");
    exigir(eh_cobertura(g, todos, 4) == 1, "todos os vertices");
    liberar_grafo(g);
    liberar_grafo(NULL);
    liberar_resultado_cobertura(NULL);
}

/* LCG de 32 bits: x <- 1664525*x + 1013904223 modulo 2^32.
 * Estado local, semente 20260909. Percurso lexicografico u<v; aresta se
 * x < 1503238553 (floor(0.35*2^32)), probabilidade aproximadamente 0.35.
 * Sem rand(), sem dependencia da implementacao da biblioteca C. */
static uint32_t proximo(uint32_t *estado) {
    *estado = *estado * UINT32_C(1664525) + UINT32_C(1013904223);
    return *estado;
}

static int gerar_arestas(int n, Aresta *a, uint32_t *estado) {
    int m = 0;
    for (int u = 0; u < n; u++) {
        for (int v = u + 1; v < n; v++) {
            if (proximo(estado) < UINT32_C(1503238553)) {
                a[m++] = (Aresta){u,v};
            }
        }
    }
    return m;
}

static void testar_instancia(int n, const Aresta *a, int m) {
    Grafo *g = criar_grafo(n, a, m);
    exigir(g != NULL, "grafo de teste");
    ResultadoCobertura b = cobertura_minima_bt(g), h = cobertura_gulosa(g);
    verificar_par(g, &b, &h);
    exigir(b.quantidade == otimo_enumeracao(n, a, m), "otimo independente");
    ResultadoCobertura repeticao = cobertura_gulosa(g);
    exigir(repeticao.status == COBERTURA_SUCESSO &&
           repeticao.quantidade == h.quantidade, "repeticao da gulosa");
    for (int i = 0; i < h.quantidade; i++) {
        exigir(repeticao.vertices[i] == h.vertices[i], "determinismo da gulosa");
    }
    liberar_resultado_cobertura(&repeticao);
    liberar_resultado_cobertura(&b);
    liberar_resultado_cobertura(&h);
    liberar_grafo(g);
}

static void verificacoes(void) {
    testar_contratos();
    int total = 0;
    /* Todos os grafos simples rotulados para cada n entre 0 e 5. */
    for (int n = 0; n <= 5; n++) {
        Aresta possiveis[10], a[10];
        int p = 0;
        for (int u = 0; u < n; u++) {
            for (int v = u + 1; v < n; v++) {
                possiveis[p++] = (Aresta){u,v};
            }
        }
        for (uint32_t mascara = 0; mascara < (UINT32_C(1) << p); mascara++) {
            int m = 0;
            for (int i = 0; i < p; i++) {
                if (mascara & (UINT32_C(1) << i)) {
                    a[m++] = possiveis[i];
                }
            }
            testar_instancia(n, a, m);
            total++;
        }
    }
    uint32_t estado = UINT32_C(20260909);
    for (int i = 0; i < 100; i++) {
        int n = 6 + i % 5;
        Aresta a[45];
        int m = gerar_arestas(n, a, &estado);
        testar_instancia(n, a, m);
    }
    printf("\nContratos OK; oraculo: %d grafos exaustivos (n=0..5) + "
           "100 reproduziveis (n=6..10).\n", total);
}

typedef ResultadoCobertura (*Algoritmo)(const Grafo *);
typedef struct {
    double microssegundos;
    unsigned repeticoes;
    int resolvido;
} Medicao;

/* clock(): unidade nominal 1/CLOCKS_PER_SEC s; resolucao efetiva pode ser
 * pior. Em Windows/MS CRT mede tempo decorrido; em ISO C/POSIX, CPU.
 * Lotes duplicam ate >= max(20 ticks nominais,10 ms), no maximo 2^20
 * chamadas. Reporta apenas o ultimo lote; nao soma lotes de calibracao.
 * Cronometra chamada completa e liberacao do resultado, com alocacoes e
 * inicializacao (inclusive a gulosa interna ao BT). Geracao, oraculo,
 * eh_cobertura e impressao ficam fora. Nao acumula resultados na memoria. */
static Medicao medir(const Grafo *g, Algoritmo algoritmo) {
    Medicao medicao = {0};
    double alvo = 20.0 / CLOCKS_PER_SEC;
    if (alvo < 0.01) {
        alvo = 0.01;
    }
    for (unsigned repeticoes = 1; ; repeticoes *= 2) {
        clock_t inicio = clock();
        int sucesso = 1;
        for (unsigned i = 0; i < repeticoes; i++) {
            ResultadoCobertura r = algoritmo(g);
            sucesso = sucesso && r.status == COBERTURA_SUCESSO;
            liberar_resultado_cobertura(&r);
        }
        clock_t fim = clock();
        exigir(sucesso, "algoritmo durante medicao");
        medicao.repeticoes = repeticoes;
        if (inicio == (clock_t)-1 || fim == (clock_t)-1 || fim < inicio) {
            return medicao;
        }
        double segundos = ((double)fim - (double)inicio) / CLOCKS_PER_SEC;
        if (segundos >= alvo) {
            medicao.resolvido = 1;
            medicao.microssegundos = segundos * 1e6 / repeticoes;
            return medicao;
        }
        if (repeticoes == (1U << 20)) {
            return medicao;
        }
    }
}

static void imprimir_medicao(Medicao m) {
    if (m.resolvido) {
        printf("%10.3f %8u", m.microssegundos, m.repeticoes);
    } else {
        printf("%10s %8u", "sem resol.", m.repeticoes);
    }
}

static void experimento(void) {
    const int tamanhos[] = {5,8,10,12,15};
    uint32_t estado = UINT32_C(20260909); /* reinicia: independente dos testes */
    printf("\nExperimento: 5 instancias/n; p~=0.35; LCG32; semente=20260909.\n");
#ifdef _WIN32
    puts("clock() no Windows/MS CRT: tempo decorrido, nao CPU.");
#else
    puts("clock() ISO C/POSIX: tempo de CPU.");
#endif
    printf("Tick nominal: %.3f us (CLOCKS_PER_SEC=%ld); resolucao efetiva "
           "pode ser pior.\n", 1e6 / CLOCKS_PER_SEC, (long)CLOCKS_PER_SEC);
    puts("Lotes adaptativos >=20 ticks e >=10 ms, limite 1048576 chamadas; "
         "incluem liberar resultado.\n"
         "Tempo medio por chamada em us; sem resol. = relogio indisponivel "
         "ou lote insuficiente.\n"
         " n inst   m exata gulosa dif  razao      BT_us   rep_BT "
         "    Gul_us  rep_Gul");
    for (int t = 0; t < 5; t++) {
        int n = tamanhos[t];
        for (int instancia = 1; instancia <= 5; instancia++) {
            Aresta a[105];
            int m = gerar_arestas(n, a, &estado);
            Grafo *g = criar_grafo(n, a, m);
            exigir(g != NULL, "grafo do experimento");
            ResultadoCobertura b = cobertura_minima_bt(g), h = cobertura_gulosa(g);
            verificar_par(g, &b, &h);
            exigir(b.quantidade == otimo_enumeracao(n, a, m),
                   "oraculo do experimento fora do tempo");
            Medicao bt, gulosa;
            /* Alterna a ordem para reduzir favorecimento sistematico. */
            if (instancia % 2) {
                bt = medir(g, cobertura_minima_bt);
                gulosa = medir(g, cobertura_gulosa);
            } else {
                gulosa = medir(g, cobertura_gulosa);
                bt = medir(g, cobertura_minima_bt);
            }
            verificar_par(g, &b, &h); /* original continua intacto */
            printf("%2d %4d %3d %5d %6d %3d ", n, instancia, m,
                   b.quantidade, h.quantidade, h.quantidade - b.quantidade);
            if (b.quantidade == 0) {
                printf("%6s ", "n/a");
            } else {
                printf("%6.3f ", (double)h.quantidade / b.quantidade);
            }
            imprimir_medicao(bt);
            printf(" ");
            imprimir_medicao(gulosa);
            puts("");
            liberar_resultado_cobertura(&b);
            liberar_resultado_cobertura(&h);
            liberar_grafo(g);
        }
    }
    puts("Razao n/a = nao se aplica (otimo zero).\n"
         "BT garante otimo, mas tem pior caso exponencial. A gulosa pode "
         "economizar tempo sacrificando qualidade; nao garante otimo nem fator 2.\n"
         "Grafos pequenos dependem da resolucao do relogio e da estrutura. "
         "Nao ha garantia de tempos crescentes nem de gulosa sempre mais rapida.\n"
         "Estes experimentos nao provam a complexidade teorica.");
}

int main(int argc, char **argv) {
    if (argc > 2 || (argc == 2 && strcmp(argv[1], "--test") != 0)) {
        fprintf(stderr, "Uso: %s [--test]\n", argv[0]);
        return EXIT_FAILURE;
    }
    exemplos();
    verificacoes();
    if (argc == 1) {
        experimento();
    }
    return EXIT_SUCCESS;
}
