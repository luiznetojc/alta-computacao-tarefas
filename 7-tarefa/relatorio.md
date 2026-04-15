# Relatório - Processamento de Lista Encadeada com Tarefas OpenMP

## Objetivo

Implementar um programa em C que percorre uma lista encadeada de nomes de arquivos fictícios e criar uma zona paralela para processar os arquivos da lista.

## Metodologia

O programa foi estruturado em três partes:

1. Construção da lista encadeada (`No`) com nomes de arquivos.
2. Entrada em uma região paralela (`#pragma omp parallel`).
3. Criação de uma tarefa por nó com `#pragma omp task`, essa tarefa com um sleep(1) para simular algum processamento.

## Uso de diretivas

- O uso de `single` é essencial para evitar criação duplicada de tarefas. Assim garantindo que cada thread tem sua propria tarefa., Sem ele as tarefas se repetem no resultado, ou seja cada thread está realizando um trabalho repetido.

- `firstprivate(no_tarefa)` garante que cada tarefa capture o valor do ponteiro no momento da criação da task. Sem ele temos problemas pois no_tarefa pode está apontando para uma localização de memoria diferente.

- `taskwait` sincroniza a thread que criou as tarefas com o término das tasks filhas. Importante pois logo após temos a liberação da memória, sem o taskwait o programa pode limpar as alocações que ainda vão ser usadas.

## Comportamento observado

Ao executar o programa, a saída mostra múltiplas threads processando arquivos diferentes, por exemplo:

- `Thread 0 processando arquivo: dados_2026_01.csv`
- `Thread 4 processando arquivo: imagem_satelite_03.png`
- `Thread 2 processando arquivo: log_execucao_77.log`

Isso confirma a execução concorrente das tasks e a distribuição dinâmica de trabalho entre threads.

## Conclusão

Para tarefas Paralelas sobre estruturas encadeadas, `single`, `firstprivate` e `taskwait` formam um conjunto fundamental de corretude:

1. `single` evita geração duplicada de tarefas.
2. `firstprivate` fixa os dados corretos para cada task.
3. `taskwait` garante sincronização antes de liberar recursos compartilhados.

```c
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
```
