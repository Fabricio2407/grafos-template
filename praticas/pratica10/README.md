# Pratica 10 — Caminho/Ciclo Hamiltoniano e TSP

Implementacao independente em C11, sem bibliotecas externas. Os arquivos
`tps.h` e `tps.c` preservam a grafia solicitada; as funcoes usam `tsp`.

## Compilacao e execucao

```sh
make
./main --test   # exemplos e verificacoes pequenas
./main          # inclui experimento n=5,8,10,12
make clean
```

No Windows, o executavel e `main.exe` (`.\main.exe` no PowerShell).
GCC e make precisam estar no PATH. Nesta validacao foram usados os programas
ja instalados em `C:\msys64\ucrt64\bin` e `C:\msys64\usr\bin`.
O Makefile usa `-O2 -Wall -Wextra -Wpedantic -Werror -std=c11` para todos
os algoritmos. `make check` executa somente os testes pequenos.

## API e memoria

Os contratos completos estao em `hamiltoniano.h` e `tps.h`:

```c
ResultadoHamiltoniano encontrar_caminho_hamiltoniano_bt(const Grafo *g);
ResultadoHamiltoniano encontrar_ciclo_hamiltoniano_bt(const Grafo *g);
ResultadoTSP tsp_forca_bruta(const GrafoTSP *g);
ResultadoTSP tsp_vizinho_mais_proximo(const GrafoTSP *g);
ResultadoTSP tsp_insercao(const GrafoTSP *g);
Status grafo_para_tsp(const Grafo *g, GrafoTSP *saida);
```

As matrizes contiguas pertencem ao chamador; os algoritmos nao as alteram.
A reducao aloca uma matriz independente, liberada com `liberar_grafo_tsp`.
Nao usar esse liberador em matrizes emprestadas, por exemplo alocadas na pilha.
Resultados possuem vetores dinamicos, liberados com
`liberar_resultado_hamiltoniano` ou `liberar_resultado_tsp` antes de sobrescrever.
Ponteiros devem apontar para regioes validas e suficientemente grandes.

Estados: `SOLUCAO`, `SEM_SOLUCAO`, `ENTRADA_INVALIDA`, `SEM_MEMORIA`, `OVERFLOW`.
Custos usam `int64_t`; zero e peso valido. Overflow e erro de representacao,
nao inexistencia matematica de uma rota. A busca exata ainda conta rotas cujo
custo excede esse tipo e encontra o otimo se houver alguma rota representavel.
Se `(n-1)!` exceder `uint64_t`, recusa a enumeracao antes de inicia-la.

Grafos vazios nao possuem caminho ou rota nesta convencao. Um vertice possui
caminho `[0]`; ciclo simples exige pelo menos tres vertices. No TSP, `[0,0]`
tem custo zero e `[0,1,0]` percorre duas vezes a mesma aresta. Essas convencoes
de rota para n=1 e n=2 nao representam ciclos simples.

## Algoritmos

- Caminho: backtracking com visitados, desfazendo o estado e tentando todas
  as origens. Ciclo: fixa 0 e exige a aresta final de retorno. O pior caso
  continua fatorial, mesmo com a poda por adjacencia; limite O(n*n!).
- TSP exato: fixa 0 e enumera todas as `(n-1)!` permutacoes incrementalmente,
  sem branch-and-bound. Avaliar cada rota do zero custaria O(n); acumular
  os custos evita recomputar as arestas comuns aos prefixos. A implementacao
  percorre n candidatos por no interno, com limite O(n*(n-1)!).
- Vizinho mais proximo: menor peso para um vertice ainda nao visitado,
  desempate por menor indice; O(n^2).
- Nearest insertion: primeiro escolhe o vertice de menor distancia a algum
  vertice da rota; depois escolhe sua melhor posicao. Empates: menor indice
  de vertice e primeira aresta na ordem da rota. O(n^3). Variacoes usam
  sinal e magnitude para evitar overflow, inclusive quando a soma de dois
  pesos excede INT64_MAX. O custo final e somado novamente pelas arestas.

Todos usam O(n) de memoria auxiliar, alem das matrizes O(n^2).
Heuristicas nao garantem otimo. Nenhuma hipotese de desigualdade triangular
e imposta aos pesos.

Na reducao, arestas originais recebem 1, pares distintos restantes recebem 2
e a diagonal recebe 0. Para n>=3 toda rota tem n parcelas >=1. Seu custo e n
se e somente se todas as parcelas correspondem a arestas originais, formando
um ciclo Hamiltoniano. Portanto, otimo exato >n prova ausencia de ciclo;
custo heuristico >n nao prova. A reducao corresponde a ciclo, nao a caminho.

## Validacao local executada em 26/10/2026

- GCC 16.2.0, MSYS2 UCRT64, Windows; compilacao pelo Makefile sem avisos.
- Compilacao adicional com as flags do workflow:
  `gcc -Wall -Wextra -Werror -std=c11 hamiltoniano.c tps.c main.c -o main_ci.exe -lm`;
  executado `main_ci.exe --test` com sucesso.
- Exemplos A/B/C: custos exatos 6/5/8; D: caminho `1 -> 0 -> 2`;
  E: otimo 80. Todas as rotas de todos os algoritmos foram verificadas.
- Todos os 1.100 grafos simples rotulados com n=0..5, incluindo desconexos:
  caminho e ciclo comparados com oraculo independente por subconjuntos;
  validacao de adjacencias, vertices, fechamento, pesos da reducao,
  equivalencia com o TSP exato, contagens e custos das heuristicas.
- Entradas invalidas, grafos pequenos, pesos negativos e zero, simetria,
  diagonal, empates, delta negativo, custos no limite e acima de int64_t,
  rotas representaveis junto de rotas com overflow, overflow do contador,
  preservacao das entradas e liberacao repetida de resultados zerados.
- Argumentos `--invalido` e `--test extra` rejeitados com codigo 1.
- Tentada compilacao com `-fsanitize=address,undefined`: o linker nao
  encontrou `-lasan` e `-lubsan`. Sanitizadores **nao foram executados**;
  a liberacao de memoria foi revisada e exercitada pelos testes, sem
  verificacao dinamica de vazamentos. Falhas de malloc nao foram injetadas.

## Medicoes reais

Uma instancia por tamanho. LCG de 32 bits:
`x = 1664525*x + 1013904223 (mod 2^32)`, semente `20261010+n`, reiniciada
por tamanho. Percorre pares `i<j` em ordem lexicografica, atribui
`1+x%100` e espelha. Os tres algoritmos recebem exatamente a mesma matriz.

Fonte: `clock()` da UCRT no Windows, tempo decorrido, `CLOCKS_PER_SEC=1000`
(tick nominal de 1 ms). Geracao, impressao e validacao fora dos tempos.
Busca exata executada uma vez por tamanho, liberacao fora do tempo.
Heuristicas: lotes dobram ate pelo menos 20 ticks e 20 ms, limite 2^20;
reporta somente o ultimo lote, incluindo alocacao e liberacao por chamada.
Tempos abaixo sao microssegundos por chamada, sem sanitizadores, `-O2`.

| n | Otimo | Vizinho | Insercao | Desvio vizinho | Desvio insercao | Rotas exatas |
|---:|---:|---:|---:|---:|---:|---:|
| 5 | 216 | 216 | 216 | 0,00% | 0,00% | 24 |
| 8 | 188 | 207 | 209 | 10,11% | 11,17% | 5.040 |
| 10 | 179 | 223 | 183 | 24,58% | 2,23% | 362.880 |
| 12 | 237 | 300 | 280 | 26,58% | 18,14% | 39.916.800 |

| n | Exata (us) | Vizinho (us) | Insercao (us) | Repeticoes vizinho | Repeticoes insercao |
|---:|---:|---:|---:|---:|---:|
| 5 | Sem resolucao | 0,187 | 0,252 | 262.144 | 131.072 |
| 8 | Sem resolucao | 0,336 | 0,443 | 65.536 | 65.536 |
| 10 | 13.000 | 0,290 | 0,443 | 131.072 | 65.536 |
| 12 | 1.298.000 | 0,381 | 0,732 | 65.536 | 65.536 |

Desvio: `100*(custo_heuristica-custo_otimo)/custo_otimo`.
Sem resolucao significa que os dois timestamps foram iguais; nao significa
tempo zero. Nao foram substituidos por estimativas. Tempos dependem da
instancia, do relogio e do ambiente; esta amostra nao e um estudo estatistico
nem demonstra complexidade assintotica. A busca de n=12 foi concluida,
sem reduzir o tamanho e sem repeticoes desnecessarias.
