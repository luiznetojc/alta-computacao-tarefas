#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define NX 256
#define NY 256
#define DX 1.0
#define DY 1.0
#define DT 0.1
#define NU 0.5
#define STEPS 500

static double u[NX][NY];
static double u_new[NX][NY];

int main(int argc, char *argv[])
{
    int threads = 4;
    if (argc > 1) threads = atoi(argv[1]);
    omp_set_num_threads(threads);

    memset(u, 0, sizeof(u));
    u[NX/2][NY/2] = 1.0;

    double t0 = omp_get_wtime();

    for (int t = 0; t < STEPS; t++) {
#pragma omp parallel for schedule(dynamic, 8)
        for (int i = 1; i < NX - 1; i++) {
            for (int j = 1; j < NY - 1; j++) {
                double lap = (u[i+1][j] - 2.0*u[i][j] + u[i-1][j]) / (DX*DX)
                           + (u[i][j+1] - 2.0*u[i][j] + u[i][j-1]) / (DY*DY);
                u_new[i][j] = u[i][j] + DT * NU * lap;
            }
        }
#pragma omp parallel for schedule(dynamic, 8)
        for (int i = 1; i < NX - 1; i++)
            for (int j = 1; j < NY - 1; j++)
                u[i][j] = u_new[i][j];
    }

    double elapsed = omp_get_wtime() - t0;

    double mx = 0.0;
    for (int i = 0; i < NX; i++)
        for (int j = 0; j < NY; j++)
            if (fabs(u[i][j]) > mx) mx = fabs(u[i][j]);

    printf("Versao: dynamic(8)       Threads: %d  Tempo: %.6f s  VelMax: %.6f\n",
           threads, elapsed, mx);
    return 0;
}
