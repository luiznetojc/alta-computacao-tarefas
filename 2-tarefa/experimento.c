#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define STEPS 100000000

double now() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000.0 + t.tv_usec / 1000.0;
}

void shuffle(size_t *array, size_t n) {
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        size_t tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
    }
}

void testar(size_t tamanho) {

    size_t elementos = tamanho / sizeof(size_t);

    size_t *vetor = malloc(elementos * sizeof(size_t));

    for (size_t i = 0; i < elementos; i++)
        vetor[i] = i;

    shuffle(vetor, elementos);

    size_t idx = 0;

    double start = now();

    for (size_t i = 0; i < STEPS; i++) {
        idx = vetor[idx];
    }

    double end = now();

    printf("Array: %zu KB | Tempo: %.2f ms\n", tamanho / 1024, end - start);

    free(vetor);
}

int main() {

    srand(time(NULL));

    size_t tamanhos[] = {
        4 * 1024,
        8 * 1024,
        16 * 1024,
        32 * 1024,
        64 * 1024,
        128 * 1024,
        256 * 1024,
        512 * 1024,
        1 * 1024 * 1024,
        2 * 1024 * 1024,
        4 * 1024 * 1024,
        8 * 1024 * 1024,
        16 * 1024 * 1024,
        32 * 1024 * 1024,
        64 * 1024 * 1024
    };

    int n = sizeof(tamanhos) / sizeof(size_t);

    for (int i = 0; i < n; i++) {
        testar(tamanhos[i]);
    }

    return 0;
}w