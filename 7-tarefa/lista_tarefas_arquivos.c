#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct No
{
	char archive[128];
	struct No *nextNode;
} No;

No *criar_no(const char *archive)
{
	No *novo = (No *)malloc(sizeof(No));
	if (novo == NULL)
	{
		perror("Erro ao alocar no");
		exit(EXIT_FAILURE);
	}

	strncpy(novo->archive, archive, sizeof(novo->archive) - 1);
	novo->archive[sizeof(novo->archive) - 1] = '\0';
	novo->nextNode = NULL;
	return novo;
}

void inserir_no_final(No **head, const char *archive)
{
	No *novo = criar_no(archive);

	if (*head == NULL)
	{
		*head = novo;
		return;
	}

	No *atual = *head;
	while (atual->nextNode != NULL)
	{
		atual = atual->nextNode;
	}
	atual->nextNode = novo;
}

void liberar_lista(No *head)
{
	while (head != NULL)
	{
		No *temp = head;
		head = head->nextNode;
		free(temp);
	}
}

int main(void)
{
	No *lista = NULL;

	const char *arquivos[] = {
		"dados_2026_01.csv",
		"relatorio_final.txt",
		"imagem_satelite_03.png",
		"backup_cliente_A.tar",
		"log_execucao_77.log",
		"metadados.bin"};
	size_t qtd_arquivos = sizeof(arquivos) / sizeof(arquivos[0]);
	for (size_t i = 0; i < qtd_arquivos; i++)
	{
		inserir_no_final(&lista, arquivos[i]);
	}

#pragma omp parallel
	{
#pragma omp single
		{
		for (No *atual = lista; atual != NULL; atual = atual->nextNode)
		{
			No *no_tarefa = atual;

			#pragma omp task firstprivate(no_tarefa)
			{
				printf("Thread %d processando arquivo: %s\n",
					   omp_get_thread_num(), no_tarefa->archive);
			}
		}

			#pragma omp taskwait
		}
	}

	liberar_lista(lista);
	return 0;
}