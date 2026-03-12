#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define VALOR 1000000000

double tempo_execucao(void (*func)(unsigned long long*), unsigned long long *vetor) {
    struct timeval start, end;

    gettimeofday(&start, NULL);
    func(vetor);
    gettimeofday(&end, NULL);

    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_usec - start.tv_usec) / 1000.0;
}

void teste1(unsigned long long *vetor) {
    unsigned long long total = 0;

    for (unsigned long long i = 0; i < VALOR; i++) {
        total += vetor[i];
    }

    printf("Total1: %llu\n", total);
}

void teste2(unsigned long long *vetor) {
    unsigned long long s1 = 0, s2 = 0;

    for (unsigned long long i = 0; i < VALOR; i += 2) {
        s1 += vetor[i];
        s2 += vetor[i + 1];
    }

    printf("Total2: %llu\n", s1 + s2);
}

void teste4(unsigned long long *vetor) {
    unsigned long long s1=0,s2=0,s3=0,s4=0;

    for (unsigned long long i = 0; i < VALOR; i += 4) {
        s1 += vetor[i];
        s2 += vetor[i+1];
        s3 += vetor[i+2];
        s4 += vetor[i+3];
    }

    printf("Total4: %llu\n", s1+s2+s3+s4);
}

void teste8(unsigned long long *vetor) {
    unsigned long long s1=0,s2=0,s3=0,s4=0,s5=0,s6=0,s7=0,s8=0;

    for (unsigned long long i = 0; i < VALOR; i += 8) {
        s1 += vetor[i];
        s2 += vetor[i+1];
        s3 += vetor[i+2];
        s4 += vetor[i+3];
        s5 += vetor[i+4];
        s6 += vetor[i+5];
        s7 += vetor[i+6];
        s8 += vetor[i+7];
    }

    printf("Total8: %llu\n", s1+s2+s3+s4+s5+s6+s7+s8);
}

void teste16(unsigned long long *vetor) {
    unsigned long long s[16] = {0};

    for (unsigned long long i = 0; i < VALOR; i += 16) {
        for (int j = 0; j < 16; j++) {
            s[j] += vetor[i + j];
        }
    }

    unsigned long long total = 0;
    for(int i=0;i<16;i++) total += s[i];

    printf("Total16: %llu\n", total);
}

int main() {

    unsigned long long *vetor = malloc(sizeof(unsigned long long) * VALOR);

    for (unsigned long long i = 0; i < VALOR; i++) {
        vetor[i] = i + 1;
    }

    printf("Teste 1 acumulador\n");
    printf("Tempo: %f ms\n\n", tempo_execucao(teste1, vetor));

    printf("Teste 2 acumuladores\n");
    printf("Tempo: %f ms\n\n", tempo_execucao(teste2, vetor));

    printf("Teste 4 acumuladores\n");
    printf("Tempo: %f ms\n\n", tempo_execucao(teste4, vetor));

    printf("Teste 8 acumuladores\n");
    printf("Tempo: %f ms\n\n", tempo_execucao(teste8, vetor));

    printf("Teste 16 acumuladores\n");
    printf("Tempo: %f ms\n\n", tempo_execucao(teste16, vetor));

    free(vetor);

    return 0;
}