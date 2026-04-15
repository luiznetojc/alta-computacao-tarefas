#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char *argv[])
{
	long long total_pontos = 100000000;
	int nthreads = omp_get_max_threads();

	if (argc > 1)
	{
		total_pontos = atoll(argv[1]);
	}

	long long *acertos_por_thread = calloc((size_t)nthreads, sizeof(long long));
	if (!acertos_por_thread)
	{
		fprintf(stderr, "Erro ao alocar vetor de acertos.\n");
		return 1;
	}

	double inicio = omp_get_wtime();

#pragma omp parallel
	{
		int tid = omp_get_thread_num();
		unsigned int seed = 42u + (unsigned int)tid * 97u;

#pragma omp for
		for (long long i = 0; i < total_pontos; i++)
		{
			double x = (double)rand_r(&seed) / RAND_MAX;
			double y = (double)rand_r(&seed) / RAND_MAX;

			if (x * x + y * y <= 1.0)
			{
				acertos_por_thread[tid]++;
			}
		}
	}

	long long total_dentro = 0;
	for (int i = 0; i < nthreads; i++)
	{
		total_dentro += acertos_por_thread[i];
	}

	double fim = omp_get_wtime();
	double pi = 4.0 * (double)total_dentro / (double)total_pontos;

	printf("Programa: rand_r + vetor + soma serial\n");
	printf("Threads: %d\n", nthreads);
	printf("Pontos: %lld\n", total_pontos);
	printf("Pi estimado: %.9f\n", pi);
	printf("Tempo: %.6f s\n", fim - inicio);

	free(acertos_por_thread);
	return 0;
}