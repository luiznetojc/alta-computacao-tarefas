#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NX 1024
#define NY 1024
#define DX 1.0
#define DY 1.0
#define DT 0.1
#define NU 0.5
#define STEPS 2000

static double u[NX][NY];
static double u_new[NX][NY];

static void step(void)
{
    for (int i = 1; i < NX - 1; i++) {
        for (int j = 1; j < NY - 1; j++) {
            double lap = (u[i+1][j] - 2.0*u[i][j] + u[i-1][j]) / (DX*DX)
                       + (u[i][j+1] - 2.0*u[i][j] + u[i][j-1]) / (DY*DY);
            u_new[i][j] = u[i][j] + DT * NU * lap;
        }
    }
    for (int i = 1; i < NX - 1; i++)
        for (int j = 1; j < NY - 1; j++)
            u[i][j] = u_new[i][j];
}

static double max_velocity(void)
{
    double mx = 0.0;
    for (int i = 0; i < NX; i++)
        for (int j = 0; j < NY; j++)
            if (fabs(u[i][j]) > mx) mx = fabs(u[i][j]);
    return mx;
}

int main(void)
{
    memset(u, 0, sizeof(u));

    printf("Fase 1: campo uniforme u=0\n");
    for (int t = 0; t < STEPS; t++) step();
    printf("  velocidade maxima apos %d passos: %.6f\n", STEPS, max_velocity());

    memset(u, 0, sizeof(u));
    u[NX/2][NY/2] = 1.0;
    printf("Fase 2: perturbacao unitaria em (%d,%d)\n", NX/2, NY/2);

    for (int t = 1; t <= STEPS; t++) {
        step();
        if (t % 400 == 0)
            printf("  passo %4d: velocidade maxima = %.6f\n", t, max_velocity());
    }

    return 0;
}
