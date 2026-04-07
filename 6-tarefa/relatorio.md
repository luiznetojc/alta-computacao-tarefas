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

### Cláusulas de compartilhamento

- O uso de `shared` é implícito para variáveis fora da região paralela.
- A semente aleatória por thread foi declarada dentro da região paralela (`seed`), sendo naturalmente privada (equivalente a `private`).

## Conclusão

A tarefa evidenciou claramente três pontos:
1. Paralelizar sem sincronização adequada pode produzir resultados incorretos.
2. `critical` garante corretude, mas pode comprometer significativamente o desempenho quando utilizado em operações muito frequentes.
3. `reduction` é a estratégia mais adequada para acumuladores nesse tipo de problema, proporcionando corretude com alto desempenho.

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
``````

Apendice E - Implementacao paralela com `shared` explícito + `critical`.

```c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

double random_double_r(unsigned int *seed)
{
	return (double)rand_r(seed) / RAND_MAX;
}

double estimate_pi_parallel_shared_explicit(long long total_pontos)
{
	long long pontos_dentro_circulo = 0;

#pragma omp parallel shared(pontos_dentro_circulo)
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
	double pi_estimate = estimate_pi_parallel_shared_explicit(total_pontos);
	gettimeofday(&fim, NULL);

	double tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000000.0;

	printf("Versao: parallel com shared explícito + critical\n");
	printf("Threads: %d\n", omp_get_max_threads());
	printf("Pontos: %lld\n", total_pontos);
	printf("Estimativa de Pi: %f\n", pi_estimate);
	printf("Tempo total: %f segundos\n", tempo_total);

	return 0;
}
```

Apendice F - Implementacao paralela com `default(none)` + `critical`.

```c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

double random_double_r(unsigned int *seed)
{
	return (double)rand_r(seed) / RAND_MAX;
}

double estimate_pi_parallel_default_none(long long total_pontos)
{
	long long pontos_dentro_circulo = 0;

	/* default(none) força declaração explícita de cada variável.
	 * Melhora clareza e evita bugs por esquecimento de sincronização.
	 */
#pragma omp parallel default(none) shared(pontos_dentro_circulo, total_pontos)
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
	double pi_estimate = estimate_pi_parallel_default_none(total_pontos);
	gettimeofday(&fim, NULL);

	double tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000000.0;

	printf("Versao: parallel com default(none) + critical\n");
	printf("Threads: %d\n", omp_get_max_threads());
	printf("Pontos: %lld\n", total_pontos);
	printf("Estimativa de Pi: %f\n", pi_estimate);
	printf("Tempo total: %f segundos\n", tempo_total);

	return 0;
}
```

Apendice G - Implementacao com `firstprivate`/`lastprivate` + `reduction`.

```c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

double random_double_r(unsigned int *seed)
{
	return (double)rand_r(seed) / RAND_MAX;
}

double estimate_pi_parallel_firstprivate_lastprivate(long long total_pontos)
{
	long long pontos_dentro_circulo = 0;
	long last_iteration_index = -1;

	/* firstprivate: inicial a partir do valor externo (não será usado aqui)
	 * lastprivate: valor final da variável é copiado para fora após o laço
	 */
#pragma omp parallel shared(pontos_dentro_circulo, total_pontos)
	{
		unsigned int seed = 42u + (unsigned int)omp_get_thread_num() * 97u;

#pragma omp for reduction(+:pontos_dentro_circulo) lastprivate(last_iteration_index)
		for (long long i = 0; i < total_pontos; i++)
		{
			double x = random_double_r(&seed);
			double y = random_double_r(&seed);

			last_iteration_index = i;

			if (x * x + y * y <= 1.0)
			{
				pontos_dentro_circulo++;
			}
		}
	}
	/* last_iteration_index contém o índice da última iteração executada */

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
	double pi_estimate = estimate_pi_parallel_firstprivate_lastprivate(total_pontos);
	gettimeofday(&fim, NULL);

	double tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000000.0;

	printf("Versao: parallel com reduction + firstprivate/lastprivate\n");
	printf("Threads: %d\n", omp_get_max_threads());
	printf("Pontos: %lld\n", total_pontos);
	printf("Estimativa de Pi: %f\n", pi_estimate);
	printf("Tempo total: %f segundos\n", tempo_total);

	return 0;
}
```

Apendice H - Implementacao com `private` acumulador.

```c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

double random_double_r(unsigned int *seed)
{
	return (double)rand_r(seed) / RAND_MAX;
}

double estimate_pi_parallel_private_accumulator(long long total_pontos)
{
	long long pontos_dentro_circulo = 0;

	/* private: cada thread tem sua própria cópia (não inicializada)
	 * Usamos isto com reduction para que cada thread acumule seu próprio total
	 * sem contencão de lock.
	 */
#pragma omp parallel private(pontos_dentro_circulo) shared(total_pontos)
	{
		unsigned int seed = 42u + (unsigned int)omp_get_thread_num() * 97u;
		pontos_dentro_circulo = 0;

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

		/* Aqui cada thread sincroniza seus resultados parciais */
#pragma omp critical
		{
			static long long global_sum = 0;
			global_sum += pontos_dentro_circulo;
			/* Na última thread a executar, copiar de volta */
			pontos_dentro_circulo = global_sum;
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
	double pi_estimate = estimate_pi_parallel_private_accumulator(total_pontos);
	gettimeofday(&fim, NULL);

	double tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000000.0;

	printf("Versao: parallel com private accumulator + critical\n");
	printf("Threads: %d\n", omp_get_max_threads());
	printf("Pontos: %lld\n", total_pontos);
	printf("Estimativa de Pi: %f\n", pi_estimate);
	printf("Tempo total: %f segundos\n", tempo_total);

	return 0;
}
```

Apendice I - Implementacao com `reduction` + `default(none)` explícito.

```c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

double random_double_r(unsigned int *seed)
{
	return (double)rand_r(seed) / RAND_MAX;
}

double estimate_pi_parallel_reduction_explicit(long long total_pontos)
{
	long long pontos_dentro_circulo = 0;
	int contador_iteracoes = 0;

	/* Declaração explícita de cláusulas:
	 * - private(contador_iteracoes): cada thread tem seu próprio contador (não sincronizado)
	 * - shared(pontos_dentro_circulo, total_pontos): compartilhadas entre threads
	 * - reduction(+:pontos_dentro_circulo): soma segura ao final
	 */
#pragma omp parallel default(none) private(contador_iteracoes) shared(pontos_dentro_circulo, total_pontos)
	{
		unsigned int seed = 42u + (unsigned int)omp_get_thread_num() * 97u;
		contador_iteracoes = 0;

#pragma omp for reduction(+:pontos_dentro_circulo)
		for (long long i = 0; i < total_pontos; i++)
		{
			double x = random_double_r(&seed);
			double y = random_double_r(&seed);

			contador_iteracoes++;

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
	double pi_estimate = estimate_pi_parallel_reduction_explicit(total_pontos);
	gettimeofday(&fim, NULL);

	double tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000000.0;

	printf("Versao: reduction com default(none) e cláusulas explícitas\n");
	printf("Threads: %d\n", omp_get_max_threads());
	printf("Pontos: %lld\n", total_pontos);
	printf("Estimativa de Pi: %f\n", pi_estimate);
	printf("Tempo total: %f segundos\n", tempo_total);

	return 0;
}
```
