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