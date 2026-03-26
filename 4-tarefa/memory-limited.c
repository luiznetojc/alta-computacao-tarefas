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
	printf("sizeof(casos): %lu\n", sizeof(casos));
	printf("total_casos: %d\n", total_casos);
	
	int max_threads = omp_get_max_threads();
	printf("Threads max: %d\n\n", max_threads);

    printf("Benchmark memory-bound\n");

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