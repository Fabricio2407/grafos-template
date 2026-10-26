#include "tps.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void exigir(int condicao, const char *mensagem) {
    if (!condicao) {
        fprintf(stderr, "Falha: %s\n", mensagem);
        exit(EXIT_FAILURE);
    }
}

static void validar_vertices(const int *v, int quantidade, int n, int fechado) {
    exigir(v && quantidade == n+fechado, "comprimento da rota");
    for (int i = 0; i < n; i++) {
        exigir(v[i] >= 0 && v[i] < n, "indice da rota");
        for (int j = 0; j < i; j++) exigir(v[i] != v[j], "vertice repetido");
    }
    if (fechado) exigir(v[n] == v[0], "fechamento da rota");
}

static void validar_h(const Grafo *g, const ResultadoHamiltoniano *r, int ciclo) {
    if (r->status == SEM_SOLUCAO) {
        exigir(!r->vertices && !r->quantidade, "resultado sem solucao vazio");
        return;
    }
    exigir(r->status == SOLUCAO, "status Hamiltoniano");
    validar_vertices(r->vertices, r->quantidade, g->n, ciclo);
    for (int i = 1; i < r->quantidade; i++)
        exigir(g->adj[(size_t)r->vertices[i-1]*g->n+r->vertices[i]], "adjacencia");
}

static void validar_tsp(const GrafoTSP *g, const ResultadoTSP *r) {
    exigir(r->status == SOLUCAO, "status TSP");
    validar_vertices(r->vertices, r->quantidade, g->n, 1);
    exigir(r->vertices[0] == 0, "origem TSP");
    int64_t custo = 0;
    for (int i = 1; i <= g->n; i++) {
        int64_t p = g->pesos[(size_t)r->vertices[i-1]*g->n+r->vertices[i]];
        exigir(custo <= INT64_MAX-p, "custo do validador representavel");
        custo += p;
    }
    exigir(custo == r->custo, "custo informado igual a soma das arestas");
}

static void mostrar(const char *nome, const int *v, int quantidade) {
    printf("  %s: ", nome);
    if (!quantidade) printf("nao existe");
    for (int i = 0; i < quantidade; i++) printf("%s%d", i ? " -> " : "", v[i]);
    putchar('\n');
}

typedef ResultadoTSP (*Algoritmo)(const GrafoTSP *);
static Algoritmo algoritmos[] = {
    tsp_forca_bruta, tsp_vizinho_mais_proximo, tsp_insercao
};
static const char *nomes[] = {"Forca bruta", "Vizinho", "Insercao"};

static void verificar_tres(const GrafoTSP *g, int64_t esperado, int imprimir) {
    for (int a = 0; a < 3; a++) {
        ResultadoTSP r = algoritmos[a](g);
        validar_tsp(g, &r);
        exigir(r.custo >= esperado && (a || r.custo == esperado), "otimo TSP");
        if (imprimir) {
            mostrar(nomes[a], r.vertices, r.quantidade);
            printf("    custo=%" PRId64 ", rotas=%" PRIu64 "\n", r.custo, r.rotas_avaliadas);
        }
        liberar_resultado_tsp(&r);
    }
}

static void exemplo(const char *nome, int n, const int arestas[][2], int m,
                    int caminho, int ciclo, int64_t custo) {
    unsigned char adj[25] = {0};
    for (int i = 0; i < m; i++) {
        int u = arestas[i][0], v = arestas[i][1];
        adj[u*n+v] = adj[v*n+u] = 1;
    }
    Grafo g = {n,adj};
    ResultadoHamiltoniano p = encontrar_caminho_hamiltoniano_bt(&g);
    ResultadoHamiltoniano c = encontrar_ciclo_hamiltoniano_bt(&g);
    validar_h(&g,&p,0); validar_h(&g,&c,1);
    exigir((p.status == SOLUCAO) == caminho && (c.status == SOLUCAO) == ciclo,
           "existencia nos exemplos");
    printf("\n%s\n", nome);
    mostrar("Caminho",p.vertices,p.quantidade);
    mostrar("Ciclo",c.vertices,c.quantidade);
    GrafoTSP t;
    exigir(grafo_para_tsp(&g,&t) == SOLUCAO, "reducao");
    verificar_tres(&t,custo,1);
    if (n >= 3) exigir((custo == n) == ciclo, "equivalencia da reducao");
    liberar_grafo_tsp(&t);
    liberar_resultado_hamiltoniano(&p); liberar_resultado_hamiltoniano(&c);
}

static void exemplos(void) {
    const int caminho[][2] = {{0,1},{1,2},{2,3},{3,4}};
    const int ciclo[][2] = {{0,1},{1,2},{2,3},{3,4},{4,0}};
    const int estrela[][2] = {{0,1},{0,2},{0,3},{0,4}};
    const int outra[][2] = {{1,0},{0,2}};
    exemplo("A: caminho de cinco vertices",5,caminho,4,1,0,6);
    exemplo("B: ciclo de cinco vertices",5,ciclo,5,1,1,5);
    exemplo("C: estrela",5,estrela,4,0,0,8);
    exemplo("D: origem diferente de zero",3,outra,2,1,0,4);
    int64_t pesos[] = {0,10,15,20, 10,0,35,25, 15,35,0,30, 20,25,30,0};
    GrafoTSP t = {4,pesos};
    puts("\nE: TSP ponderado");
    verificar_tres(&t,80,1);
}

/* Oraculo independente dos BT: programacao dinamica por subconjuntos,
 * apenas nos testes n<=5. dp[S][v] indica caminho cobrindo S e terminando
 * em v. Para ciclo, inicializa somente 0 e verifica aresta de fechamento. */
static int oraculo(const Grafo *g, int ciclo) {
    if (!g->n || (ciclo && g->n < 3)) return 0;
    unsigned char dp[32][5] = {{0}};
    for (int v = 0; v < (ciclo ? 1 : g->n); v++) dp[1U<<v][v] = 1;
    unsigned limite = 1U<<g->n;
    for (unsigned s = 1; s < limite; s++)
        for (int v = 0; v < g->n; v++)
            if (dp[s][v])
                for (int w = 0; w < g->n; w++)
                    if (!(s & (1U<<w)) && g->adj[v*g->n+w])
                        dp[s | (1U<<w)][w] = 1;
    for (int v = 0; v < g->n; v++)
        if (dp[limite-1][v] && (!ciclo || g->adj[v*g->n])) return 1;
    return 0;
}

static void exaustivos(void) {
    int total = 0;
    for (int n = 0; n <= 5; n++) {
        unsigned limite = 1U << (n*(n-1)/2);
        for (unsigned mascara = 0; mascara < limite; mascara++) {
            unsigned char adj[25] = {0}, copia[25];
            unsigned bit = 0;
            for (int i = 0; i < n; i++)
                for (int j = i+1; j < n; j++,bit++)
                    adj[i*n+j] = adj[j*n+i] = (unsigned char)((mascara>>bit)&1U);
            memcpy(copia,adj,sizeof(adj));
            Grafo g = {n,adj};
            ResultadoHamiltoniano p = encontrar_caminho_hamiltoniano_bt(&g);
            ResultadoHamiltoniano c = encontrar_ciclo_hamiltoniano_bt(&g);
            validar_h(&g,&p,0); validar_h(&g,&c,1);
            exigir((p.status == SOLUCAO) == oraculo(&g,0), "oraculo caminho");
            exigir((c.status == SOLUCAO) == oraculo(&g,1), "oraculo ciclo");
            GrafoTSP t;
            exigir(grafo_para_tsp(&g,&t) == SOLUCAO, "reducao exaustiva");
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    exigir(t.pesos[i*n+j] == (i == j ? 0 : adj[i*n+j] ? 1 : 2),
                           "pesos da reducao");
            ResultadoTSP r = tsp_forca_bruta(&t);
            if (n) {
                validar_tsp(&t,&r);
                uint64_t f = 1;
                for (int i = 2; i < n; i++) f *= (uint64_t)i;
                exigir(r.rotas_avaliadas == f, "contagem pequena");
                if (n >= 3) exigir((r.custo == n) == (c.status == SOLUCAO), "reducao exata");
                for (int a = 1; a < 3; a++) {
                    ResultadoTSP h = algoritmos[a](&t);
                    validar_tsp(&t,&h);
                    exigir(h.custo >= r.custo, "heuristica >= otimo");
                    liberar_resultado_tsp(&h);
                }
            } else exigir(r.status == SEM_SOLUCAO, "TSP vazio");
            exigir(!memcmp(copia,adj,sizeof(adj)), "entrada Hamiltoniana preservada");
            liberar_resultado_tsp(&r); liberar_grafo_tsp(&t);
            liberar_resultado_hamiltoniano(&p); liberar_resultado_hamiltoniano(&c);
            total++;
        }
    }
    printf("\nOraculo DP e reducao: %d grafos (todos para n=0..5), OK.\n",total);
}

static void status_tres(const GrafoTSP *g, Status status) {
    for (int a = 0; a < 3; a++) {
        ResultadoTSP r = algoritmos[a](g);
        exigir(r.status == status && !r.vertices && !r.quantidade && !r.custo,
               "contrato de erro/vazio TSP");
        liberar_resultado_tsp(&r); liberar_resultado_tsp(&r);
    }
}

static void contratos(void) {
    unsigned char ruim[] = {0,1,0,0};
    Grafo invalidos[] = {{-1,NULL},{INT_MAX,NULL},{2,NULL},{2,ruim}};
    for (size_t i = 0; i <= sizeof(invalidos)/sizeof(*invalidos); i++) {
        const Grafo *g = i == sizeof(invalidos)/sizeof(*invalidos) ? NULL : &invalidos[i];
        ResultadoHamiltoniano p = encontrar_caminho_hamiltoniano_bt(g);
        ResultadoHamiltoniano c = encontrar_ciclo_hamiltoniano_bt(g);
        exigir(p.status == ENTRADA_INVALIDA && c.status == ENTRADA_INVALIDA &&
               !p.vertices && !c.vertices && !p.quantidade && !c.quantidade, "Hamiltoniano invalido");
        GrafoTSP t;
        exigir(grafo_para_tsp(g,&t) == ENTRADA_INVALIDA && !t.pesos && !t.n, "reducao invalida");
        liberar_resultado_hamiltoniano(&p); liberar_resultado_hamiltoniano(&p);
        liberar_resultado_hamiltoniano(&c);
    }
    ruim[0] = 1;
    exigir(!grafo_valido(&(Grafo){2,ruim}), "laco");
    ruim[0] = 0; ruim[1] = ruim[2] = 2;
    exigir(!grafo_valido(&(Grafo){2,ruim}), "adjacencia nao booleana");
    exigir(grafo_para_tsp(&(Grafo){0,NULL},NULL) == ENTRADA_INVALIDA, "saida nula");
    status_tres(NULL,ENTRADA_INVALIDA);
    status_tres(&(GrafoTSP){-1,NULL},ENTRADA_INVALIDA);
    status_tres(&(GrafoTSP){INT_MAX,NULL},ENTRADA_INVALIDA);
    status_tres(&(GrafoTSP){2,NULL},ENTRADA_INVALIDA);
    status_tres(&(GrafoTSP){0,NULL},SEM_SOLUCAO);
    int64_t pesos[25] = {0};
    GrafoTSP t = {2,pesos};
    pesos[1] = -1; pesos[2] = -1;
    status_tres(&t,ENTRADA_INVALIDA);
    pesos[1] = 1; pesos[2] = 2;
    status_tres(&t,ENTRADA_INVALIDA);
    pesos[1] = pesos[2] = 0; pesos[0] = 1;
    status_tres(&t,ENTRADA_INVALIDA);
    pesos[0] = 0; pesos[1] = pesos[2] = INT64_MAX;
    status_tres(&t,OVERFLOW);
    pesos[1] = pesos[2] = INT64_MAX/2;
    verificar_tres(&t,INT64_MAX-1,0);
    t.n = 1;
    verificar_tres(&t,0,0);
    /* Empates e pesos zero: vizinho ascendente, insercao na primeira
     * aresta, logo ordem reversa. Testa tambem preservacao da matriz. */
    t.n = 5;
    memset(pesos,0,sizeof(pesos));
    for (int valor = 0; valor <= 1; valor++) {
        for (int i = 0; i < 5; i++)
            for (int j = 0; j < 5; j++) pesos[i*5+j] = i == j ? 0 : valor;
        int64_t copia[25]; memcpy(copia,pesos,sizeof(pesos));
        verificar_tres(&t,5*valor,0);
        for (int a = 1; a < 3; a++) {
            ResultadoTSP r = algoritmos[a](&t);
            validar_tsp(&t,&r);
            for (int i = 1; i < 5; i++) exigir(r.vertices[i] == (a == 1 ? i : 5-i), "desempate");
            liberar_resultado_tsp(&r);
        }
        exigir(!memcmp(pesos,copia,sizeof(pesos)), "matriz TSP preservada");
    }
    /* Algumas rotas excedem int64_t; outras ainda possuem otimo 4. */
    int64_t mistos[] = {0,1,INT64_MAX,1, 1,0,1,INT64_MAX,
                       INT64_MAX,1,0,1, 1,INT64_MAX,1,0};
    verificar_tres(&(GrafoTSP){4,mistos},4,0);
    /* Inserir 3 entre 2 e 1 reduz o custo em 95: delta negativo. */
    int64_t negativos[] = {0,2,3,4, 2,0,100,4, 3,100,0,1, 4,4,1,0};
    ResultadoTSP inserida = tsp_insercao(&(GrafoTSP){4,negativos});
    validar_tsp(&(GrafoTSP){4,negativos},&inserida);
    const int rota_inserida[] = {0,2,3,1,0};
    exigir(inserida.custo == 10 &&
           !memcmp(inserida.vertices,rota_inserida,sizeof(rota_inserida)), "insercao com delta negativo");
    liberar_resultado_tsp(&inserida);
    /* Custo exatamente INT64_MAX e deltas positivos maiores que ele. */
    int64_t grandes[] = {0,INT64_MAX/2,INT64_MAX/2,
                        INT64_MAX/2,0,1, INT64_MAX/2,1,0};
    verificar_tres(&(GrafoTSP){3,grandes},INT64_MAX,0);
    int64_t excesso[] = {0,INT64_MAX,INT64_MAX,INT64_MAX,0,INT64_MAX,INT64_MAX,INT64_MAX,0};
    status_tres(&(GrafoTSP){3,excesso},OVERFLOW);
    int64_t muitos[22*22] = {0};
    ResultadoTSP r = tsp_forca_bruta(&(GrafoTSP){22,muitos});
    exigir(r.status == OVERFLOW && !r.vertices && !r.rotas_avaliadas, "overflow fatorial");
    liberar_resultado_tsp(&r);
    liberar_resultado_tsp(NULL); liberar_resultado_hamiltoniano(NULL);
    liberar_grafo_tsp(NULL);
    puts("Contratos, pesos zero, empates, overflow e liberacao repetida: OK.");
}

typedef struct { double us; unsigned repeticoes; int resolvido; } Medicao;
static double segundos(clock_t inicio, clock_t fim) {
    exigir(inicio != (clock_t)-1 && fim != (clock_t)-1 && fim >= inicio, "relogio disponivel");
    return ((double)fim-(double)inicio)/CLOCKS_PER_SEC;
}

static Medicao medir_heuristica(const GrafoTSP *g, Algoritmo algoritmo) {
    double alvo = 20.0/CLOCKS_PER_SEC;
    if (alvo < 0.02) alvo = 0.02;
    for (unsigned repeticoes = 1; ; repeticoes *= 2) {
        clock_t inicio = clock();
        for (unsigned i = 0; i < repeticoes; i++) {
            ResultadoTSP r = algoritmo(g);
            liberar_resultado_tsp(&r);
        }
        clock_t fim = clock();
        double s = segundos(inicio,fim);
        if (s >= alvo || repeticoes == (1U<<20))
            return (Medicao){s*1e6/repeticoes,repeticoes,s >= alvo};
    }
}

static void imprimir_tempo(Medicao m) {
    if (m.resolvido) printf("%.3f",m.us);
    else printf("sem_resolucao");
}

static void experimento(void) {
    const int tamanhos[] = {5,8,10,12};
    const uint64_t contagens[] = {24,5040,362880,39916800};
    puts("\nExperimento: LCG32 x=1664525*x+1013904223 mod 2^32;\n"
         "semente=20261010+n reiniciada por tamanho; pares i<j em ordem\n"
         "lexicografica, peso=1+x%100, espelhado. Uma instancia por n.");
#ifdef _WIN32
    puts("clock() no Windows/UCRT: tempo decorrido; unidade abaixo: us/chamada.");
#else
    puts("clock() ISO C: tempo de CPU; unidade abaixo: us/chamada.");
#endif
    printf("CLOCKS_PER_SEC=%ld. Heuristicas: ultimo lote >=20 ticks e >=20 ms,\n"
           "limite 1048576 repeticoes; incluem liberacao. Exata: uma chamada\n"
           "por n, liberacao fora do tempo; sem resolucao se abaixo de 1 tick.\n"
           "Geracao, validacao e impressao fora dos tempos; mesma compilacao -O2.\n",
           (long)CLOCKS_PER_SEC);
    puts("n otimo vizinho insercao desvio_v% desvio_i% exata_us vizinho_us insercao_us rotas rep_v rep_i");
    for (int t = 0; t < 4; t++) {
        int n = tamanhos[t];
        int64_t pesos[144] = {0}, copia[144];
        uint32_t estado = UINT32_C(20261010)+(uint32_t)n;
        for (int i = 0; i < n; i++)
            for (int j = i+1; j < n; j++) {
                estado = estado*UINT32_C(1664525)+UINT32_C(1013904223);
                pesos[i*n+j] = pesos[j*n+i] = 1+estado%100;
            }
        memcpy(copia,pesos,sizeof(pesos));
        GrafoTSP g = {n,pesos};
        clock_t inicio = clock();
        ResultadoTSP exata = tsp_forca_bruta(&g);
        clock_t fim = clock();
        double s = segundos(inicio,fim);
        Medicao bt = {s*1e6,1,fim > inicio};
        ResultadoTSP v = tsp_vizinho_mais_proximo(&g), ins = tsp_insercao(&g);
        validar_tsp(&g,&exata); validar_tsp(&g,&v); validar_tsp(&g,&ins);
        exigir(exata.rotas_avaliadas == contagens[t], "(n-1)! no experimento");
        exigir(v.custo >= exata.custo && ins.custo >= exata.custo, "qualidade das heuristicas");
        Medicao mv = medir_heuristica(&g,tsp_vizinho_mais_proximo);
        Medicao mi = medir_heuristica(&g,tsp_insercao);
        exigir(!memcmp(copia,pesos,sizeof(pesos)), "entrada do experimento preservada");
        printf("%d %" PRId64 " %" PRId64 " %" PRId64 " %.2f %.2f ",n,exata.custo,v.custo,ins.custo,
               100.0*(double)(v.custo-exata.custo)/(double)exata.custo,
               100.0*(double)(ins.custo-exata.custo)/(double)exata.custo);
        imprimir_tempo(bt); putchar(' '); imprimir_tempo(mv); putchar(' '); imprimir_tempo(mi);
        printf(" %" PRIu64 " %u %u\n",exata.rotas_avaliadas,mv.repeticoes,mi.repeticoes);
        fflush(stdout);
        liberar_resultado_tsp(&exata); liberar_resultado_tsp(&v); liberar_resultado_tsp(&ins);
    }
    puts("Tempos dependem da instancia, relogio e ambiente. Forca bruta garante\n"
         "o otimo; heuristicas podem produzir rotas subotimas. Custo heuristico\n"
         ">n na reducao nao demonstra ausencia de ciclo Hamiltoniano.");
}

int main(int argc, char **argv) {
    if (argc > 2 || (argc == 2 && strcmp(argv[1],"--test"))) {
        fprintf(stderr,"Uso: %s [--test]\n",argv[0]);
        return EXIT_FAILURE;
    }
    exemplos(); contratos(); exaustivos();
    if (argc == 1) experimento();
    return EXIT_SUCCESS;
}
