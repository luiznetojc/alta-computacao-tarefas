## Relatório - Estimativa de Pi com OpenMP

## Objetivo

Implementar e comparar versões sequencial e paralelas para estimar o valor de π utilizando o método de Monte Carlo, avaliando corretude e desempenho com OpenMP.

## Metodologia

Foram consideradas quatro versões:

- **Sequencial**: algoritmo original em um único fluxo de execução (Apêndice A).
- **Paralela incorreta (sem proteção)**: utiliza `#pragma omp parallel` + `#pragma omp for`, mas atualiza uma variável compartilhada sem sincronização, gerando condição de corrida (Apêndice B).
- **Paralela com `critical`**: utiliza seção crítica para proteger a atualização da contagem de pontos dentro do círculo (Apêndice C).
- **Paralela com `reduction`**: utiliza redução para somar contagens locais de cada thread de forma eficiente e correta (Apêndice D).

**Detalhes do experimento:**

- Pontos amostrados: `50.000.000`
- Número de threads nas versões paralelas: `8` (`OMP_NUM_THREADS=8`)
- Compilação: `gcc -O2 -Wall -Wextra -std=c11 -fopenmp`

## Resultados

| Versão                               | Estimativa de π | Tempo (s) |
| ------------------------------------ | --------------- | --------- |
| Sequencial                           | 3.141979        | 0.296331  |
| Paralela sem proteção (incorreta)    | 0.550567        | 0.173590  |
| Paralela com `critical`              | 3.141310        | 2.392673  |
| Paralela com `reduction`             | 3.141310        | 0.036503  |

## Análise

A versão paralela sem proteção apresentou valor incorreto de π (0.550567), evidenciando uma condição de corrida na variável compartilhada `pontos_dentro_circulo`.

A versão com `critical` corrige o problema, porém o desempenho piora significativamente devido à alta contenção das threads elas precisam disputar acesso à seção crítica a cada incremento.

A versão com `reduction` corrige o problema do `critical`  e foi a mais eficiente, com speedup aproximado de:

$$
S = \frac{T_{sequencial}}{T_{reduction}} = \frac{0.296331}{0.036503} \approx 8.12
$$

Esse resultado é coerente com o esperado para um problema com iterações independentes, no qual a redução evita gargalos de sincronização fina.

## Testes e comportamento

- Usar `shared` sem sincronização resultou em valores incorretos devido a multiplas escritas
- Substituir variáveis críticas por `private` eliminou interferência entre threads, mas ainda não conseguimos o resultado ideal pois perdermos as alterações na variável de cada thread
- `firstprivate` e `lastprivate` não trouxe ganhos relevantes neste caso.

## Conclusão

A tarefa evidenciou claramente três pontos:

1. Paralelizar sem sincronização adequada pode produzir resultados incorretos.
2. `critical` garante corretude, mas pode comprometer significativamente o desempenho quando utilizado em operações muito frequentes.
3. `reduction` foi a solução mais perfomàtica para esse problema, proporcionando corretude com alto desempenho.

## Apêndice

Apendice A - Implementacao sequencial.

```c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

double random_double()
{
	return (double)rand() / RAND_MAX;
}

double estimate_pi(long long total_pontos)
{
	long long pontos_dentro_circulo = 0;
	for (long long i = 0; i < total_pontos; i++)
	{
		double x = random_double();
		double y = random_double();

		if (x * x + y * y <= 1.0)
		{
			pontos_dentro_circulo++;
		}
	}
	return (double)pontos_dentro_circulo / total_pontos * 4.0;
}

int main(int argc, char *argv[])
{
	long long total_pontos = 100000000;
	if (argc > 1)
	{
		total_pontos = atoll(argv[1]);
	}

	srand(42);
	struct timeval inicio, fim;
	gettimeofday(&inicio, NULL);

	double pi_estimate = estimate_pi(total_pontos);

	printf("Versao: sequencial\n");
	printf("Pontos: %lld\n", total_pontos);
	printf("Estimativa de Pi: %f\n", pi_estimate);

	gettimeofday(&fim, NULL);
	double tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000000.0;
	printf("Tempo total: %f segundos\n", tempo_total);

	return 0;
}
```

Apendice B - Implementacao paralela sem protecao (errada).

```c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

double random_double_r(unsigned int *seed)
{
	return (double)rand_r(seed) / RAND_MAX;
}

double estimate_pi_parallel_wrong(long long total_pontos)
{
	long long pontos_dentro_circulo = 0;

#pragma omp parallel
	{
		unsigned int seed = 42u + (unsigned int)omp_get_thread_num() * 97u;

#pragma omp for
		for (long long i = 0; i < total_pontos; i++)
		{
			double x = random_double_r(&seed);
			double y = random_double_r(&seed);

			if (x * x + y * y <= 1.0)
			{
				pontos_dentro_circulo++;
			}
		}
	}

	return (double)pontos_dentro_circulo / total_pontos * 4.0;
}

int main(int argc, char *argv[])
{
	long long total_pontos = 100000000;
	if (argc > 1)
	{
		total_pontos = atoll(argv[1]);
	}

	struct timeval inicio, fim;
	gettimeofday(&inicio, NULL);
	double pi_estimate = estimate_pi_parallel_wrong(total_pontos);
	gettimeofday(&fim, NULL);

	double tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000000.0;

	printf("Versao: parallel for SEM protecao (errada)\n");
	printf("Threads: %d\n", omp_get_max_threads());
	printf("Pontos: %lld\n", total_pontos);
	printf("Estimativa de Pi: %f\n", pi_estimate);
	printf("Tempo total: %f segundos\n", tempo_total);

	return 0;
}
```

Apendice C - Implementacao paralela com `critical`.

```c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

double random_double_r(unsigned int *seed)
{
	return (double)rand_r(seed) / RAND_MAX;
}

double estimate_pi_parallel_critical(long long total_pontos)
{
	long long pontos_dentro_circulo = 0;

#pragma omp parallel
	{
		unsigned int seed = 42u + (unsigned int)omp_get_thread_num() * 97u;

#pragma omp for
		for (long long i = 0; i < total_pontos; i++)
		{
			double x = random_double_r(&seed);
			double y = random_double_r(&seed);

			if (x * x + y * y <= 1.0)
			{
#pragma omp critical
				{
					pontos_dentro_circulo++;
				}
			}
		}
	}

	return (double)pontos_dentro_circulo / total_pontos * 4.0;
}

int main(int argc, char *argv[])
{
	long long total_pontos = 100000000;
	if (argc > 1)
	{
		total_pontos = atoll(argv[1]);
	}

	struct timeval inicio, fim;
	gettimeofday(&inicio, NULL);
	double pi_estimate = estimate_pi_parallel_critical(total_pontos);
	gettimeofday(&fim, NULL);

	double tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000000.0;

	printf("Versao: parallel + critical\n");
	printf("Threads: %d\n", omp_get_max_threads());
	printf("Pontos: %lld\n", total_pontos);
	printf("Estimativa de Pi: %f\n", pi_estimate);
	printf("Tempo total: %f segundos\n", tempo_total);

	return 0;
}
```

Apendice D - Implementacao paralela com `reduction`.

```c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

double random_double_r(unsigned int *seed)
{
	return (double)rand_r(seed) / RAND_MAX;
}

double estimate_pi_parallel_reduction(long long total_pontos)
{
	long long pontos_dentro_circulo = 0;

#pragma omp parallel
	{
		unsigned int seed = 42u + (unsigned int)omp_get_thread_num() * 97u;

#pragma omp for reduction(+ : pontos_dentro_circulo)
		for (long long i = 0; i < total_pontos; i++)
		{
			double x = random_double_r(&seed);
			double y = random_double_r(&seed);

			if (x * x + y * y <= 1.0)
			{
				pontos_dentro_circulo++;
			}
		}
	}

	return (double)pontos_dentro_circulo / total_pontos * 4.0;
}

int main(int argc, char *argv[])
{
	long long total_pontos = 100000000;
	if (argc > 1)
	{
		total_pontos = atoll(argv[1]);
	}

	struct timeval inicio, fim;
	gettimeofday(&inicio, NULL);
	double pi_estimate = estimate_pi_parallel_reduction(total_pontos);
	gettimeofday(&fim, NULL);

	double tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000000.0;

	printf("Versao: parallel + reduction\n");
	printf("Threads: %d\n", omp_get_max_threads());
	printf("Pontos: %lld\n", total_pontos);
	printf("Estimativa de Pi: %f\n", pi_estimate);
	printf("Tempo total: %f segundos\n", tempo_total);

	return 0;
}
```