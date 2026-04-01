## **Relatório – Paralelização da Contagem de Números Primos**

## **Objetivo**

Implementar um programa em C para contar números primos entre 2 e N, e comparar o desempenho entre versões sequencial e paralela utilizando OpenMP.

## **Metodologia**

Foram desenvolvidas três versões:

- **Sequencial**: execução simples do laço. (Apêndice A)
- **Paralela (static)**: uso de `#pragma omp parallel for` (escalonamento padrão estático). (Apêndice B)
- **Paralela (dynamic)**: uso de `schedule(dynamic)` para melhor distribuição de carga. (Apêndice C)

Observação: as versões paralelas utilizam a cláusula `reduction` na variável contadora de primos, garantindo atualização segura em ambiente concorrente.

## **Resultados**
Para n=1.000.000:

| Versão             | Primos | Tempo (s) |
| ------------------ | ------ | --------- |
| Sequencial         | 78498  | 23.39     |
| Paralela (static)  | 78498  | 4.62      |
| Paralela (dynamic) | 78498  | 3.05      |

## **Análise**

Todas as versões produziram o mesmo resultado, indicando que a paralelização manteve a **corretude** e não apresentou condições de corrida, devido ao uso da cláusula `reduction`, que garante atualização segura da variável compartilhada.
Houve ganho significativo de desempenho:
- aproximadamente 5x mais rápido (static)
- aproximadamente 7,6x mais rápido (dynamic)
A versão dynamic apresentou melhor desempenho devido ao balanceamento de carga mais eficiente, já que números maiores exigem mais iterações no teste de primo, aumentando o custo computacional de algumas iterações.

## **Conclusão**

A paralelização proporcionou ganho significativo de desempenho sem comprometer a corretude. O uso de escalonamento dinâmico mostrou-se mais eficiente devido à melhor distribuição do trabalho entre as threads. O problema mostrou-se adequado à paralelização por possuir iterações independentes.

## Apêndice
Apêndice A - Implementação sequencial.
```c 
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

static int eh_primo(int x)
{
	for (int d = 2; d < x; d++)
	{
		if (x % d == 0)
		{
			return 0;
		}
	}

	return 1;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Uso: %s <n>\n", argv[0]);
		return 1;
	}

	int n = atoi(argv[1]);
	int quantidade_primos = 0;
	struct timeval inicio, fim;
	gettimeofday(&inicio, NULL);

	for (int i = 2; i <= n; i++)
	{
		if (eh_primo(i))
		{
			quantidade_primos++;
		}
	}

	gettimeofday(&fim, NULL);
	double tempo_segundos =
		(fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000000.0;

	printf("Quantidade de primos entre 2 e %d: %d\n", n, quantidade_primos);
	printf("Tempo de execucao: %.6f s\n", tempo_segundos);
	return 0;
}
```

Apêndice B - Implementação Paralela Static(default).
```c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

static int eh_primo(int x)
{
	if (x < 2)
	{
		return 0;
	}

	for (int d = 2; d < x; d++)
	{
		if (x % d == 0)
		{
			return 0;
		}
	}

	return 1;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Uso: %s <n>\n", argv[0]);
		return 1;
	}

	int n = atoi(argv[1]);
	int quantidade_primos = 0;
	double inicio = omp_get_wtime();

#pragma omp parallel for reduction(+ : quantidade_primos)
	for (int i = 2; i <= n; i++)
	{
		if (eh_primo(i))
		{
			quantidade_primos++;
		}
	}

	double fim = omp_get_wtime();
	double tempo_segundos = fim - inicio;

	printf("Quantidade de primos entre 2 e %d: %d\n", n, quantidade_primos);
	printf("Tempo de execucao: %.6f s\n", tempo_segundos);
	return 0;
}
```

Apêndice C - Implementação Paralela Dynamic
```c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

static int eh_primo(int x) {
    if (x < 2) {
        return 0;
    }

    for (int d = 2; d < x; d++) {
        if (x % d == 0) {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <n>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int quantidade_primos = 0;
    double inicio = omp_get_wtime();

    #pragma omp parallel for reduction(+:quantidade_primos) schedule(dynamic)
    for (int i = 2; i <= n; i++) {
        if (eh_primo(i)) {
            quantidade_primos++;
        }
    }

    double fim = omp_get_wtime();
    double tempo_segundos = fim - inicio;

    printf("Quantidade de primos entre 2 e %d: %d\n", n, quantidade_primos);
    printf("Tempo de execucao: %.6f s\n", tempo_segundos);
    return 0;
}
```
