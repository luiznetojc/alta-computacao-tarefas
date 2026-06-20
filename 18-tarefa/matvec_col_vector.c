#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv)
{
	int rank, size;
	int M, N;
	double *A = NULL, *x = NULL, *y = NULL;
	double *local_A = NULL, *local_x = NULL, *local_y = NULL;
	int local_N;
	double start_time, end_time, local_time, max_time;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (argc != 3)
	{
		if (rank == 0)
			fprintf(stderr, "Uso: %s <M> <N>\n", argv[0]);
		MPI_Finalize();
		return 1;
	}

	M = atoi(argv[1]);
	N = atoi(argv[2]);

	if (N % size != 0)
	{
		if (rank == 0)
			fprintf(stderr, "Erro: Numero de colunas N (%d) deve ser multiplo do numero de processos (%d).\n", N, size);
		MPI_Finalize();
		return 1;
	}

	local_N = N / size;

	// Alocacao
	local_A = (double *)malloc(M * local_N * sizeof(double));
	local_x = (double *)malloc(local_N * sizeof(double));
	local_y = (double *)malloc(M * sizeof(double));

	if (rank == 0)
	{
		A = (double *)malloc(M * N * sizeof(double));
		y = (double *)malloc(M * sizeof(double));
		x = (double *)malloc(N * sizeof(double));

		// Inicializacao
		for (int i = 0; i < M; i++)
			for (int j = 0; j < N; j++)
				A[i * N + j] = 1.0;
		for (int i = 0; i < N; i++)
			x[i] = 1.0;
	}

	MPI_Datatype col_type;
	MPI_Type_vector(M, local_N, N, MPI_DOUBLE, &col_type);
	MPI_Type_commit(&col_type);

	MPI_Barrier(MPI_COMM_WORLD);
	start_time = MPI_Wtime();

	// Scatter do vetor x (segmentos correspondentes)
	MPI_Scatter(x, local_N, MPI_DOUBLE, local_x, local_N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// Scatter das colunas da matriz A
	// O buffer de recebimento local_A armazena os blocos de colunas de forma contígua!
	MPI_Scatter(A, 1, col_type, local_A, M * local_N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// Produto matriz-vetor local (contribuicao parcial)
	for (int i = 0; i < M; i++)
	{
		local_y[i] = 0.0;
		for (int j = 0; j < local_N; j++)
		{
			// local_A contem M linhas de tamanho local_N, de forma contígua
			local_y[i] += local_A[i * local_N + j] * local_x[j];
		}
	}

	// Reduce das contribuicoes parciais
	if (rank == 0)
	{
		MPI_Reduce(local_y, y, M, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Reduce(local_y, NULL, M, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	}

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
			printf("%f\n", max_time);
		else
			printf("-1.0\n");
		free(A);
		free(y);
		free(x);
	}

	MPI_Type_free(&col_type);
	free(local_A);
	free(local_x);
	free(local_y);
	MPI_Finalize();
	return 0;
}
