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
