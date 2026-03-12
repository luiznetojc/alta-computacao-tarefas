#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define VALOR 1000000000  // 100 milhões

int main() {

    unsigned long long *vetor = malloc(sizeof(unsigned long long) * VALOR);

    for (unsigned long long i = 0; i < VALOR; i++) {
        vetor[i] = i + 1;
    }

    struct timeval start, end;

    gettimeofday(&start, NULL);

    unsigned long long total = 0;

    for (unsigned long long i = 0; i < VALOR; i++) {
        total += vetor[i];
    }

    gettimeofday(&end, NULL);

    double tempo = (end.tv_sec - start.tv_sec) * 1000.0 +
                   (end.tv_usec - start.tv_usec) / 1000.0;

    printf("Tempo normal: %f ms\n", tempo);

    gettimeofday(&start, NULL);

    unsigned long long soma1 = 0, soma2 = 0, soma3 = 0, soma4 = 0;

    for (unsigned long long i = 0; i < VALOR; i += 4) {
        soma1 += vetor[i];
        soma2 += vetor[i + 1];
        soma3 += vetor[i + 2];
        soma4 += vetor[i + 3];
    }

    unsigned long long total_paralelo = soma1 + soma2 + soma3 + soma4;

    gettimeofday(&end, NULL);

    double tempo_paralelo = (end.tv_sec - start.tv_sec) * 1000.0 +
                            (end.tv_usec - start.tv_usec) / 1000.0;

    printf("Tempo paralelo: %f ms\n", tempo_paralelo);

    printf("Total: %llu\n", total);
    printf("Total paralelo: %llu\n", total_paralelo);

    free(vetor);

    return 0;
}