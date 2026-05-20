#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define LIDER 0
#define WORKTAG 1
#define DIETAG 2

// Funcao rudimentar de primalidade
int is_prime(int n)
{
	if (n <= 1)
		return 0;
	if (n <= 3)
		return 1;
	if (n % 2 == 0 || n % 3 == 0)
		return 0;
	for (int i = 5; i * i <= n; i += 6)
	{
		if (n % i == 0 || n % (i + 2) == 0)
			return 0;
	}
	return 1;
}

int main(int argc, char **argv)
{
	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int NUM_MAX = 5000000;
	int CHUNK_SIZE = 100000;

	if (argc > 1)
		NUM_MAX = atoi(argv[1]);
	if (argc > 2)
		CHUNK_SIZE = atoi(argv[2]);

	double start_time = MPI_Wtime();

	if (size == 1)
	{
		long global_count = 0;
		for (int i = 1; i <= NUM_MAX; i++)
		{
			if (is_prime(i))
			{
				global_count++;
			}
		}
		double end_time = MPI_Wtime();
		printf("Limite: %d | Sequencial (1 Processo)\n", NUM_MAX);
		printf("Total de primos: %ld\n", global_count);
		printf("Tempo total: %f segundos\n", end_time - start_time);
	}
	else
	{
		if (rank == LIDER)
		{
			int tasks_sent = 0;
			int active_workers = size - 1;
			long global_count = 0;

			int current_start = 1;

			// Enviar a primeira tarefa para todos os trabalhadores
			for (int i = 1; i < size; i++)
			{
				if (current_start <= NUM_MAX)
				{
					int range[2];
					range[0] = current_start;
					range[1] = current_start + CHUNK_SIZE - 1;
					if (range[1] > NUM_MAX)
						range[1] = NUM_MAX;

					MPI_Send(range, 2, MPI_INT, i, WORKTAG, MPI_COMM_WORLD);
					current_start += CHUNK_SIZE;
					tasks_sent++;
				}
				else
				{
					MPI_Send(NULL, 0, MPI_INT, i, DIETAG, MPI_COMM_WORLD);
					active_workers--;
				}
			}

			// Receber resultados e enviar o resto das tarefas
			while (active_workers > 0)
			{
				long result;
				MPI_Status status;

				// Recebe o numero de primos contados por algum trabalhador
				MPI_Recv(&result, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				global_count += result;

				if (current_start <= NUM_MAX)
				{
					int range[2];
					range[0] = current_start;
					range[1] = current_start + CHUNK_SIZE - 1;
					if (range[1] > NUM_MAX)
						range[1] = NUM_MAX;

					MPI_Send(range, 2, MPI_INT, status.MPI_SOURCE, WORKTAG, MPI_COMM_WORLD);
					current_start += CHUNK_SIZE;
					tasks_sent++;
				}
				else
				{
					MPI_Send(NULL, 0, MPI_INT, status.MPI_SOURCE, DIETAG, MPI_COMM_WORLD);
					active_workers--;
				}
			}

			double end_time = MPI_Wtime();
			printf("Limite: %d | Tamanho do chunk: %d | Processos: %d (1 Lider, %d Trabalhadores)\n", NUM_MAX, CHUNK_SIZE, size, size - 1);
			printf("Total de primos: %ld\n", global_count);
			printf("Tarefas processadas: %d\n", tasks_sent);
			printf("Tempo total: %f segundos\n", end_time - start_time);
		}
		else
		{
			// Trabalhador
			while (1)
			{
				int range[2];
				MPI_Status status;

				MPI_Recv(range, 2, MPI_INT, LIDER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

				if (status.MPI_TAG == DIETAG)
				{
					break; // Finaliza
				}

				long local_count = 0;
				for (int i = range[0]; i <= range[1]; i++)
				{
					if (is_prime(i))
					{
						local_count++;
					}
				}

				MPI_Send(&local_count, 1, MPI_LONG, LIDER, 0, MPI_COMM_WORLD);
			}
		}
	}

	MPI_Finalize();
	return 0;
}