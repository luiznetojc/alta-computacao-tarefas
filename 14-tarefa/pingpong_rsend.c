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
		printf("Versão: MPI_Rsend (Ready Send)\n");
		printf("Tamanho(Bytes)\tTempo(s)\n");
	}

	// O MPI_Rsend requer estritamente que a OP de recepção do destino (MPI_Recv) já tenha
	// sido postada (chamada) antes. Como num Ping-pong em loop fechado é arriscado ocorrer race-condition
	// a gente simula esse preparamento da recepção usando um token vazio.
	for (int msg_size = 8; msg_size <= MAX_SIZE; msg_size *= 2)
	{
		MPI_Barrier(MPI_COMM_WORLD);

		double start_time = MPI_Wtime();

		for (int i = 0; i < EXCHANGES; i++)
		{
			if (rank == 0)
			{
				// Rank 0 recebe o OK que rank 1 já tá esperando a mensagem
				MPI_Recv(NULL, 0, MPI_BYTE, 1, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Rsend(buffer, msg_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);

				// Rank 0 fala que agora já tá preparado pra receber e espera ele em MPI_Recv
				// Não é o envio mais elegante (adiciona overhead na medicao),
				// mas protege a validacão de MPI_Rsend perfeitamente em ping-pongs blockantes.
				MPI_Send(NULL, 0, MPI_BYTE, 1, 99, MPI_COMM_WORLD);
				MPI_Recv(buffer, msg_size, MPI_BYTE, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			else if (rank == 1)
			{
				// Manda o OK falando pro Rsend começar
				MPI_Send(NULL, 0, MPI_BYTE, 0, 99, MPI_COMM_WORLD);
				MPI_Recv(buffer, msg_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				MPI_Recv(NULL, 0, MPI_BYTE, 0, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Rsend(buffer, msg_size, MPI_BYTE, 0, 1, MPI_COMM_WORLD);
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