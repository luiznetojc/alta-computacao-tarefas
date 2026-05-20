#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ROWS 4000 // Reduzido para evitar deadlock na rede
#define COLS 4000
#define STEPS 100
#define C 0.1

int main(int argc, char **argv)
{
	int rank, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int local_R = ROWS / size;
	double *u = (double *)calloc((local_R + 2) * COLS, sizeof(double));
	double *u_new = (double *)calloc((local_R + 2) * COLS, sizeof(double));

	for (int r = 1; r <= local_R; r++)
	{
		for (int c = 0; c < COLS; c++)
		{
			u[r * COLS + c] = (rank == size / 2 && r == local_R / 2 && c == COLS / 2) ? 100.0 : 20.0;
		}
	}

	MPI_Request reqs[4];

	MPI_Barrier(MPI_COMM_WORLD);
	double start = MPI_Wtime();

	for (int t = 0; t < STEPS; t++)
	{
		int req_count = 0;

		if (rank > 0)
		{
			MPI_Irecv(&u[0 * COLS], COLS, MPI_DOUBLE, rank - 1, 1, MPI_COMM_WORLD, &reqs[req_count++]);
			MPI_Isend(&u[1 * COLS], COLS, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &reqs[req_count++]);
		}
		else
		{
			for (int c = 0; c < COLS; c++)
				u[0 * COLS + c] = u[1 * COLS + c];
		}

		if (rank < size - 1)
		{
			MPI_Irecv(&u[(local_R + 1) * COLS], COLS, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &reqs[req_count++]);
			MPI_Isend(&u[local_R * COLS], COLS, MPI_DOUBLE, rank + 1, 1, MPI_COMM_WORLD, &reqs[req_count++]);
		}
		else
		{
			for (int c = 0; c < COLS; c++)
				u[(local_R + 1) * COLS + c] = u[local_R * COLS + c];
		}

		for (int r = 2; r <= local_R - 1; r++)
		{
			for (int c = 1; c < COLS - 1; c++)
			{
				double up = u[(r - 1) * COLS + c];
				double down = u[(r + 1) * COLS + c];
				double left = u[r * COLS + c - 1];
				double right = u[r * COLS + c + 1];
				double center = u[r * COLS + c];
				u_new[r * COLS + c] = center + C * (up + down + left + right - 4.0 * center);
			}
		}

		if (req_count > 0)
			MPI_Waitall(req_count, reqs, MPI_STATUSES_IGNORE);

		if (local_R >= 1)
		{
			int r = 1;
			for (int c = 1; c < COLS - 1; c++)
			{
				double up = u[(r - 1) * COLS + c];
				double down = u[(r + 1) * COLS + c];
				double left = u[r * COLS + c - 1];
				double right = u[r * COLS + c + 1];
				double center = u[r * COLS + c];
				u_new[r * COLS + c] = center + C * (up + down + left + right - 4.0 * center);
			}
			if (local_R > 1)
			{
				r = local_R;
				for (int c = 1; c < COLS - 1; c++)
				{
					double up = u[(r - 1) * COLS + c];
					double down = u[(r + 1) * COLS + c];
					double left = u[r * COLS + c - 1];
					double right = u[r * COLS + c + 1];
					double center = u[r * COLS + c];
					u_new[r * COLS + c] = center + C * (up + down + left + right - 4.0 * center);
				}
			}
		}

		for (int r = 1; r <= local_R; r++)
		{
			for (int c = 1; c < COLS - 1; c++)
			{
				u[r * COLS + c] = u_new[r * COLS + c];
			}
		}
	}

	double end = MPI_Wtime();

	if (rank == 0)
	{
		printf("Versao 2D Atualizada: MPI_Isend / MPI_Irecv + Sobreposicao C/ MPI_Test(like)\n");
		printf("Tempo total: %f segundos\n", end - start);
	}

	free(u);
	free(u_new);
	MPI_Finalize();
	return 0;
}
