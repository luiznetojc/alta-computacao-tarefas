## Relatorio - Estimativa estocastica de Pi com OpenMP

## Objetivo

Implementar duas estrategias de acumulacao paralela para estimar o valor de pi por Monte Carlo:

- acumulacao em variavel global com `#pragma omp critical`
- acumulacao em vetor compartilhado (uma posicao por thread) com soma serial ao final

Em seguida, repetir os testes substituindo `rand()` por `rand_r()` e comparar os quatro programas.

## Metodologia

Programas implementados:

- `pi_rand_critical.c`: `rand()` + acumulacao global protegida por `critical`
- `pi_rand_array.c`: `rand()` + vetor por thread + soma serial
- `pi_randr_critical.c`: `rand_r()` + acumulacao global protegida por `critical`
- `pi_randr_array.c`: `rand_r()` + vetor por thread + soma serial

Configuracao de teste:

- Pontos: 50.000.000
- Threads: 8 (`OMP_NUM_THREADS=8`)
- Compilacao: `make`

## Resultados

| Programa | Pi estimado | Tempo (s) |
| --- | --- | --- |
| rand + critical | ~3.1412 (variavel entre execucoes) | 0.097621 (media de 5 execucoes) |
| rand + vetor | ~3.1388 (variavel entre execucoes) | 0.144370 (media de 5 execucoes) |
| rand_r + critical | 3.141781360 | 0.112575 (media de 5 execucoes) |
| rand_r + vetor | 3.141781360 | 0.146602 (media de 5 execucoes) |

## Analise

A comparacao mostra dois fenomenos principais de memoria compartilhada:

1. `rand()` usa estado global da biblioteca C. Em paralelo, isso cria disputa sobre a mesma regiao de memoria (ou ate corrida de dados, dependendo da implementacao), o que degrada a qualidade/estabilidade da sequencia e introduz variacao no valor estimado de pi.
2. Nas versoes com vetor, cada thread atualiza repetidamente `acertos_por_thread[tid]`. Como as posicoes do vetor ficam lado a lado na mesma linha de cache, ocorre invalidação frequente entre caches de diferentes nucleos. Isso e falso compartilhamento.

Sobre os quatro programas:

- `rand + critical`: alem do custo da regiao critica no final de cada thread, o gerador global `rand()` ainda e um ponto de disputa.
- `rand + vetor`: remove `critical`, mas continua com estado global do `rand()` e ainda adiciona falso compartilhamento no vetor; por isso ficou mais lento.
- `rand_r + critical`: cada thread usa `seed` privado e evita disputa no gerador. Como a regiao critica executa uma unica soma por thread, o custo de sincronizacao e baixo.
- `rand_r + vetor`: elimina `critical`, mas sofre falso compartilhamento por atualizacoes concorrentes em posicoes adjacentes do vetor, ficando mais lento.

## Conclusao

Comparando os quatro programas, a expectativa e:

- pior caso observado: versoes com vetor sem padding, por falso compartilhamento
- melhores resultados observados: versoes com acumulacao local e soma final em `critical` (uma vez por thread)

Trocar `rand()` por `rand_r()` melhorou a estabilidade numerica (mesma estimativa em repeticoes), porque cada thread passa a ter estado privado. A diferenca de desempenho entre `critical` e vetor, neste experimento, foi dominada pelo falso compartilhamento no vetor compartilhado.

## Apendice

Os codigos-fonte completos estao nos arquivos da pasta `8-tarefa`.