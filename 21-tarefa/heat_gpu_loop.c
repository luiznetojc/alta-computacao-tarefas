#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

#define PI acos(-1.0)
#define LINE "--------------------\n"

void initial_value(const int n, const double dx, const double length, double * restrict u);
void zero(const int n, double * restrict u);
void solve(const int n, const double alpha, const double dx, const double dt, const double * restrict u, double * restrict u_tmp);
double solution(const double t, const double x, const double y, const double alpha, const double length);
double l2norm(const int n, const double * restrict u, const int nsteps, const double dt, const double alpha, const double dx, const double length);

int main(int argc, char *argv[]) {
  double start = omp_get_wtime();
  int n = 1000;
  int nsteps = 10;

  if (argc == 3) {
    n = atoi(argv[1]);
    if (n < 0) {
      fprintf(stderr, "Error: n must be positive\n");
      exit(EXIT_FAILURE);
    }
    nsteps = atoi(argv[2]);
    if (nsteps < 0) {
      fprintf(stderr, "Error: nsteps must be positive\n");
      exit(EXIT_FAILURE);
    }
  }

  double alpha = 0.1;
  double length = 1000.0;
  double dx = length / (n+1);
  double dt = 0.5 / nsteps;
  double r = alpha * dt / (dx * dx);

  printf("\n MMS heat equation GPU Target Teams Loop\n\n");
  printf(LINE);
  printf(" Grid size: %d x %d\n", n, n);
  printf(" Steps: %d\n", nsteps);

  double *u     = malloc(sizeof(double)*n*n);
  double *u_tmp = malloc(sizeof(double)*n*n);
  double *tmp;

  initial_value(n, dx, length, u);
  zero(n, u_tmp);

  double tic = omp_get_wtime();
  
  #pragma omp target data map(to: u[0:n*n]) map(alloc: u_tmp[0:n*n])
  {
    for (int t = 0; t < nsteps; ++t) {
      solve(n, alpha, dx, dt, u, u_tmp);
      tmp = u;
      u = u_tmp;
      u_tmp = tmp;
    }
    #pragma omp target update from(u[0:n*n])
  }

  double toc = omp_get_wtime();

  double norm = l2norm(n, u, nsteps, dt, alpha, dx, length);
  double stop = omp_get_wtime();

  printf("Results\n\n");
  printf("Error (L2norm): %E\n", norm);
  printf("Solve time (s): %lf\n", toc-tic);
  printf("Total time (s): %lf\n", stop-start);
  printf(LINE);

  free(u);
  free(u_tmp);
}

void initial_value(const int n, const double dx, const double length, double * restrict u) {
  double y = dx;
  for (int j = 0; j < n; ++j) {
    double x = dx;
    for (int i = 0; i < n; ++i) {
      u[i+j*n] = sin(PI * x / length) * sin(PI * y / length);
      x += dx;
    }
    y += dx;
  }
}

void zero(const int n, double * restrict u) {
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      u[i+j*n] = 0.0;
    }
  }
}

void solve(const int n, const double alpha, const double dx, const double dt, const double * restrict u, double * restrict u_tmp) {
  const double r = alpha * dt / (dx * dx);
  const double r2 = 1.0 - 4.0*r;

  #pragma omp target teams loop collapse(2) map(to: u[0:n*n]) map(from: u_tmp[0:n*n])
  for (int j = 0; j < n; ++j) {
    for (int i = 0; i < n; ++i) {
      u_tmp[i+j*n] =  r2 * u[i+j*n] +
      r * ((i < n-1) ? u[i+1+j*n] : 0.0) +
      r * ((i > 0)   ? u[i-1+j*n] : 0.0) +
      r * ((j < n-1) ? u[i+(j+1)*n] : 0.0) +
      r * ((j > 0)   ? u[i+(j-1)*n] : 0.0);
    }
  }
}

double solution(const double t, const double x, const double y, const double alpha, const double length) {
  return exp(-2.0*alpha*PI*PI*t/(length*length)) * sin(PI*x/length) * sin(PI*y/length);
}

double l2norm(const int n, const double * restrict u, const int nsteps, const double dt, const double alpha, const double dx, const double length) {
  double time = dt * (double)nsteps;
  double l2norm = 0.0;
  double y = dx;
  for (int j = 0; j < n; ++j) {
    double x = dx;
    for (int i = 0; i < n; ++i) {
      double answer = solution(time, x, y, alpha, length);
      l2norm += (u[i+j*n] - answer) * (u[i+j*n] - answer);
      x += dx;
    }
    y += dx;
  }
  return sqrt(l2norm);
}
