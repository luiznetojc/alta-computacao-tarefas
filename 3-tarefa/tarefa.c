
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

struct timeval start, end;

double timeFormater(){
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_usec - start.tv_usec) / 1000.0;
}

void leibniz(long n) 
{
	double pi = 0.0;
	for (long i = 0; i < n; i++) {

		if (i % 2 == 0) 
		{
			pi += 1.0 / (2 * i + 1);
			continue;
		} 
		pi -= 1.0 / (2 * i + 1);
		
	}
	pi *= 4;
	printf("Pi aproximado: %.15f\n", pi);
}
int main()
{
	long n_testes[5] = {1000000, 10000000, 100000000, 1000000000, 10000000000};
	for (int i = 0; i < 5; i++) {
		printf("Calculando pi para n = %ld\n", n_testes[i]);
		gettimeofday(&start, NULL);
		leibniz(n_testes[i]);
		gettimeofday(&end, NULL);
		printf("Tempo de execução para n = %ld: %.2f ms\n", n_testes[i], timeFormater());
	}
	return 0;
}