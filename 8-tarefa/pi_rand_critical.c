#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char *argv[])
{
	long long total_pontos = 100000000;
	long long total_dentro = 0;

	if (argc > 1)
	{
		total_pontos = atoll(argv[1]);
	}

	srand(42);
	double inicio = omp_get_wtime();

#pragma omp parallel
	{
		long long local_dentro = 0;

#pragma omp for
		for (long long i = 0; i < total_pontos; i++)
		{
			double x = (double)rand() / RAND_MAX;
			double y = (double)rand() / RAND_MAX;

			if (x * x + y * y <= 1.0)
			{
				local_dentro++;
			}
		}

#pragma omp critical
		{
			total_dentro += local_dentro;
		}
	}

	double fim = omp_get_wtime();
	double pi = 4.0 * (double)total_dentro / (double)total_pontos;

	printf("Programa: rand + critical\n");
	printf("Threads: %d\n", omp_get_max_threads());
	printf("Pontos: %lld\n", total_pontos);
	printf("Pi estimado: %.9f\n", pi);
	printf("Tempo: %.6f s\n", fim - inicio);

	return 0;
}