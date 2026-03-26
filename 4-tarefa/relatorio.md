## Relatório – Memory-bound e CPU-bound

### Objetivo

O objetivo desta tarefa foi desenvolver programas em C para avaliar o comportamento dos programas com bastante escrita na memória e atingir o Memory bound, e com uso excessivo de calculo para atingir o CPU bound, utilizando a biblioteca OpenMP para paralelizar a execução. A análise busca entender como o aumento do número de threads impacta o desempenho em diferentes tipos de gargalo.

### Implementação

Para o caso _memory-bound_, foi implementado um programa simples de soma entre vetores (`c[i] = a[i] + b[i]`), repetido várias vezes, a cada passo: 2 leituras + 1 escrita por elemento e pouca computação.

Para o caso _CPU-bound_, foi implementado um programa com múltiplas iterações aritméticas por elemento  reduzindo o impacto da memória e concentrando o custo na CPU.

Ambos os programas utilizam:
- OpenMP com `#pragma omp parallel for`
- Warm-up antes da medição
- Múltiplas execuções (média)
- Checksum para evitar otimização do compilador

### Resultados

#### Memory-bound

|Threads|Banda (GB/s)|Speedup|
|---|---|---|
|1|~82|1.00|
|2|~150|~1.85|
|4|~150|~1.83|
|8|~148|~1.80|

**Observação:** resultados consistentes nos três cenários testados.

#### CPU-bound

**Caso 2 (cpu_iters = 400)**

|Threads|Tempo (s)|Speedup|Eficiência|
|---|---|---|---|
|1|4.15|1.00|1.00|
|2|2.11|1.97|0.98|
|4|1.08|3.82|0.95|
|8|0.54|7.61|0.95|
|16|0.50|8.30|0.52|

**Observação:** Resultados similares foram observados nos demais casos, com pequenas variações dependendo de _cpu_iters_.

### **Análise**

Os resultados demonstram claramente a diferença entre aplicações _memory-bound_ e _CPU-bound_.

No caso _memory-bound_, o desempenho escala apenas até aproximadamente 2 threads, atingindo cerca de 150 GB/s de banda. Após esse ponto, não há ganho significativo, pois o sistema atinge o limite físico da largura de banda de memória.

No caso _CPU-bound_, o comportamento é diferente: o speed-up cresce quase linearmente até 8 threads, indicando bom aproveitamento dos núcleos. A eficiência permanece alta (~95%), com baixo overhead de paralelização. A partir de 16 threads, o ganho diminui e a eficiência cai (~50%), devido à limitação de recursos da CPU.

### **Conclusão**

Os experimentos mostram que a eficiência da paralelização depende diretamente do tipo de gargalo. Aplicações _memory-bound_ são limitadas pela largura de banda da memória e saturam rapidamente, enquanto aplicações _CPU-bound_apresentam melhor escalabilidade até o limite dos núcleos disponíveis.

## Apendice
Apendice A - Memory Bound
```c 
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    long long n;
    int mem_passes;
} MemCase;

#define REPEAT 5

static double run_kernel(long long n, int mem_passes, double *a, double *b, double *c)
{
    double start = omp_get_wtime();

    for (int p = 0; p < mem_passes; p++)
    {
#pragma omp parallel for schedule(static)
        for (long long i = 0; i < n; i++)
        {
            c[i] = a[i] + b[i];
        }

        double *tmp = a;
        a = b;
        b = c;
        c = tmp;
    }

    double end = omp_get_wtime();
    return end - start;
}

static void init_arrays(long long n, double *a, double *b, double *c)
{
#pragma omp parallel for schedule(static)
    for (long long i = 0; i < n; i++)
    {
        a[i] = (double)(i % 1000) * 0.5;
        b[i] = (double)(i % 1000) * 1.5;
        c[i] = 0.0;
    }
}

static double checksum(long long n, double *b)
{
    double sum = 0.0;

#pragma omp parallel for reduction(+ : sum)
    for (long long i = 0; i < n; i++)
    {
        sum += b[i];
    }

    return sum;
}

int main(void)
{
    MemCase casos[] = {
        {120000000LL, 10},
        {240000000LL, 8},
        {320000000LL, 6},
    };

    int total_casos = sizeof(casos) / sizeof(casos[0]);

    int max_threads = omp_get_max_threads();

    printf("Benchmark memory-bound\n");
    printf("Threads max: %d\n\n", max_threads);

    for (int c = 0; c < total_casos; c++)
    {
        long long n = casos[c].n;
        int passes = casos[c].mem_passes;

        printf("Caso %d: N=%lld, passes=%d\n", c + 1, n, passes);
        printf("threads | tempo(s) | banda(GB/s) | speedup\n");

        double *a = aligned_alloc(64, n * sizeof(double));
        double *b = aligned_alloc(64, n * sizeof(double));
        double *c_arr = aligned_alloc(64, n * sizeof(double));

        if (!a || !b || !c_arr)
        {
            printf("Erro de memoria\n");
            return 1;
        }

        init_arrays(n, a, b, c_arr);

        double baseline = 0.0;

        for (int t = 1; t <= max_threads; t *= 2)
        {
            omp_set_num_threads(t);

            double total_time = 0.0;

            run_kernel(n, passes, a, b, c_arr);

            for (int r = 0; r < REPEAT; r++)
            {
                double elapsed = run_kernel(n, passes, a, b, c_arr);
                total_time += elapsed;
            }

            double avg_time = total_time / REPEAT;

            if (t == 1)
                baseline = avg_time;

            double bytes = (double)passes * n * 3.0 * sizeof(double);
            double bandwidth = (bytes / avg_time) / 1e9;
            double speedup = baseline / avg_time;

            double chk = checksum(n, b);

            printf("%7d | %8.4f | %10.2f | %7.2f (chk=%.2f)\n",
                   t, avg_time, bandwidth, speedup, chk);
        }

        free(a);
        free(b);
        free(c_arr);

        printf("\n");
    }

    return 0;
}
```
Apendice B - CPU Bound
```c

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    long long n;
    int cpu_iters;
} CpuCase;

#define REPEAT 5
#define MAX_THREADS_TEST 32

static double run_kernel(long long n, int cpu_iters, double *c)
{
    double start = omp_get_wtime();

#pragma omp parallel for schedule(static)
    for (long long i = 0; i < n; i++)
    {
        double x = (double)i;
        for (int k = 0; k < cpu_iters; k++)
        {
            x = x * 1.0000001 + 0.0000003;
            x = x - (x * 0.00000005);
        }

        c[i] = x;
    }

    double end = omp_get_wtime();
    return end - start;
}

static double checksum(long long n, double *c)
{
    double sum = 0.0;

#pragma omp parallel for reduction(+ : sum)
    for (long long i = 0; i < n; i++)
    {
        sum += c[i];
    }

    return sum;
}

int main(void)
{
    CpuCase casos[] = {
        {5000000LL, 150},
        {5000000LL, 400},
        {12000000LL, 250},
    };

    int total_casos = sizeof(casos) / sizeof(casos[0]);

    int max_threads = omp_get_max_threads();

    printf("Benchmark CPU-bound\n");
    printf("Max threads OpenMP: %d\n", max_threads);
    printf("Testando ate %d threads\n\n", MAX_THREADS_TEST);

    for (int c = 0; c < total_casos; c++)
    {
        long long n = casos[c].n;
        int iters = casos[c].cpu_iters;

        printf("Caso %d: N=%lld, cpu_iters=%d\n", c + 1, n, iters);
        printf("threads | tempo(s) | speedup | eficiencia\n");

        double *arr = aligned_alloc(64, n * sizeof(double));
        if (!arr)
        {
            printf("Erro de memoria\n");
            return 1;
        }

        double baseline = 0.0;

        for (int t = 1; t <= MAX_THREADS_TEST; t *= 2)
        {
            if (t > max_threads * 2)
                break;

            omp_set_num_threads(t);

            double total_time = 0.0;

            run_kernel(n, iters, arr);

            for (int r = 0; r < REPEAT; r++)
            {
                total_time += run_kernel(n, iters, arr);
            }

            double avg_time = total_time / REPEAT;

            if (t == 1)
                baseline = avg_time;

            double speedup = baseline / avg_time;
            double eficiencia = speedup / t;

            double chk = checksum(n, arr);

            printf("%7d | %8.4f | %7.2f | %9.2f (chk=%.2e)\n",
                   t, avg_time, speedup, eficiencia, chk);
        }

        free(arr);
        printf("\n");
    }

    return 0;
}
```