#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

static int eh_primo(int x) {
    if (x < 2) {
        return 0;
    }

    for (int d = 2; d < x; d++) {
        if (x % d == 0) {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <n>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int quantidade_primos = 0;
    double inicio = omp_get_wtime();

    #pragma omp parallel for reduction(+:quantidade_primos) schedule(dynamic)
    for (int i = 2; i <= n; i++) {
        if (eh_primo(i)) {
            quantidade_primos++;
        }
    }

    double fim = omp_get_wtime();
    double tempo_segundos = fim - inicio;

    printf("Quantidade de primos entre 2 e %d: %d\n", n, quantidade_primos);
    printf("Tempo de execucao: %.6f s\n", tempo_segundos);
    return 0;
}
