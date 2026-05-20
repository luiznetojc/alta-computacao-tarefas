#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE (1024 * 1024)
#define EXCHANGES 1000

int main(int argc, char **argv)
{
	int rank, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (size != 2)
	{
		if (rank == 0)
			printf("Este programa deve ser executado com exatamente 2 processos.\n");
		MPI_Finalize();
		return 1;
	}

	char *buffer = (char *)malloc(MAX_SIZE);

	if (rank == 0)
	{
		printf("Versão: MPI_Ssend (Synchronous Send)\n");
		printf("Tamanho(Bytes)\tTempo(s)\n");
	}

	for (int msg_size = 8; msg_size <= MAX_SIZE; msg_size *= 2)
	{
		MPI_Barrier(MPI_COMM_WORLD);

		double start_time = MPI_Wtime();

		for (int i = 0; i < EXCHANGES; i++)
		{
			if (rank == 0)
			{
				MPI_Ssend(buffer, msg_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
				MPI_Recv(buffer, msg_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			else if (rank == 1)
			{
				MPI_Recv(buffer, msg_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Ssend(buffer, msg_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
			}
		}

		double end_time = MPI_Wtime();

		if (rank == 0)
		{
			double time_per_msg = (end_time - start_time) / (2.0 * EXCHANGES);
			printf("%d\t\t%e\n", msg_size, time_per_msg);
		}
	}

	free(buffer);
	MPI_Finalize();
	return 0;
}
