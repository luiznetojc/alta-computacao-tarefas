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

	srand(42); // Seed para resultados reprodutíveis
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
