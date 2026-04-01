#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

static int eh_primo(int x)
{
	for (int d = 2; d < x; d++)
	{
		if (x % d == 0)
		{
			return 0;
		}
	}

	return 1;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Uso: %s <n>\n", argv[0]);
		return 1;
	}

	int n = atoi(argv[1]);
	int quantidade_primos = 0;
	struct timeval inicio, fim;
	gettimeofday(&inicio, NULL);

	for (int i = 2; i <= n; i++)
	{
		if (eh_primo(i))
		{
			quantidade_primos++;
		}
	}

	gettimeofday(&fim, NULL);
	double tempo_segundos =
		(fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000000.0;

	printf("Quantidade de primos entre 2 e %d: %d\n", n, quantidade_primos);
	printf("Tempo de execucao: %.6f s\n", tempo_segundos);
	return 0;
}
