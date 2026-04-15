#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Node
{
	int value;
	int thread_id;
	int task_id;
	struct Node *next;
} Node;

typedef struct
{
	Node *head;
	int count;
	omp_lock_t lock;
} LinkedList;

static void list_init(LinkedList *list)
{
	list->head = NULL;
	list->count = 0;
	omp_init_lock(&list->lock);
}

static void list_insert(LinkedList *list, int value, int thread_id, int task_id)
{
	Node *node = (Node *)malloc(sizeof(Node));
	if (node == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	node->value = value;
	node->thread_id = thread_id;
	node->task_id = task_id;

	omp_set_lock(&list->lock);
//#pragma omp critical (list_insert)
	//{
		node->next = list->head;
		list->head = node;
		list->count++;
	//}
	omp_unset_lock(&list->lock);
}

static void list_print_summary(LinkedList *list, int list_id)
{
	printf("Lista %d: %d elementos\n", list_id, list->count);
}

static void list_destroy(LinkedList *list)
{
	Node *current = list->head;
	while (current != NULL)
	{
		Node *next = current->next;
		free(current);
		current = next;
	}
	omp_destroy_lock(&list->lock);
}

int main(int argc, char **argv)
{
	LinkedList lists[2];
	int n;
	int total_inserted;
	int total_processed = 0;

	if (argc != 2)
	{
		fprintf(stderr, "Uso: %s <N>\n", argv[0]);
		return EXIT_FAILURE;
	}

	n = atoi(argv[1]);
	if (n <= 0)
	{
		fprintf(stderr, "N deve ser maior que 0.\n");
		return EXIT_FAILURE;
	}

	list_init(&lists[0]);
	list_init(&lists[1]);
	double inicio = omp_get_wtime();
#pragma omp parallel num_threads(2) shared(lists, total_processed, n)
	{
		int tid = omp_get_thread_num();
		unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(tid * 2654435761u);

#pragma omp for schedule(static)
		for (int task_id = 0; task_id < n; task_id++)
		{
			int target_list = (int)(rand_r(&seed) % 2);
			int value = (int)(rand_r(&seed) % 1000000);

			list_insert(&lists[target_list], value, tid, task_id);

	//	#pragma omp atomic
		//		total_processed++;
			
		}
	}
	double fim = omp_get_wtime();
	double tempo_segundos = fim - inicio;
	total_inserted = lists[0].count + lists[1].count;
	printf("Insercoes solicitadas: %d\n", n);
	printf("Insercoes realizadas : %d\n", total_inserted);
	printf("Tarefas processadas  : %d\n", total_processed);
	list_print_summary(&lists[0], 0);
	list_print_summary(&lists[1], 1);
	printf("Tempo de execucao: %.6f segundos\n", tempo_segundos);
	if (total_inserted != n)
	{
		fprintf(stderr, "Erro: total de insercoes difere de N.\n");
	}

	list_destroy(&lists[0]);
	list_destroy(&lists[1]);

	return EXIT_SUCCESS;
}
