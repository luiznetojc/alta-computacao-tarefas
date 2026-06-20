#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv)
{
	int rank, size;
	int M, N;
	double *A = NULL, *x = NULL, *y = NULL;
	double *local_A = NULL, *local_y = NULL;
	int local_M;
	double start_time, end_time, local_time, max_time;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (argc != 3)
	{
		if (rank == 0)
		{
			fprintf(stderr, "Uso: %s <M> <N>\n", argv[0]);
		}
		MPI_Finalize();
		return 1;
	}

	M = atoi(argv[1]);
	N = atoi(argv[2]);

	if (M % size != 0)
	{
		if (rank == 0)
		{
			fprintf(stderr, "Erro: Numero de linhas M (%d) deve ser multiplo do numero de processos (%d).\n", M, size);
		}
		MPI_Finalize();
		return 1;
	}

	local_M = M / size;

	// Alocacao
	local_A = (double *)malloc(local_M * N * sizeof(double));
	local_y = (double *)malloc(local_M * sizeof(double));
	x = (double *)malloc(N * sizeof(double));

	if (rank == 0)
	{
		A = (double *)malloc(M * N * sizeof(double));
		y = (double *)malloc(M * sizeof(double));

		// Inicializacao
		for (int i = 0; i < M; i++)
		{
			for (int j = 0; j < N; j++)
			{
				A[i * N + j] = 1.0;
			}
		}
		for (int i = 0; i < N; i++)
		{
			x[i] = 1.0;
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	start_time = MPI_Wtime();

	// Broadcast do vetor x
	MPI_Bcast(x, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// Scatter das linhas da matriz A
	MPI_Scatter(A, local_M * N, MPI_DOUBLE, local_A, local_M * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// Produto matriz-vetor local
	for (int i = 0; i < local_M; i++)
	{
		local_y[i] = 0.0;
		for (int j = 0; j < N; j++)
		{
			local_y[i] += local_A[i * N + j] * x[j];
		}
	}

	// Gather dos resultados em y
	MPI_Gather(local_y, local_M, MPI_DOUBLE, y, local_M, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	end_time = MPI_Wtime();
	local_time = end_time - start_time;

	MPI_Reduce(&local_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	if (rank == 0)
	{
		int validation = 1;
		for (int i = 0; i < M; i++)
		{
			if (y[i] != (double)N)
			{
				validation = 0;
				break;
			}
		}
		if (validation)
		{
			printf("%f\n", max_time);
		}
		else
		{
			printf("-1.0\n");
		}
		free(A);
		free(y);
	}

	free(x);
	free(local_A);
	free(local_y);
	MPI_Finalize();
	return 0;
}
