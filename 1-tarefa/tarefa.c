#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

struct timeval start, end;

double timeFormater(){
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_usec - start.tv_usec) / 1000.0;
}

void multiplicacaoPorLinhas(int linhas, int colunas, int **A, int *x, int *y)
{
    printf("MultiplicacaoPorLinhas\n");
    gettimeofday(&start, NULL);
    for (int i = 0; i < linhas; i++) 
	{
        int soma = 0;
        for (int j = 0; j < colunas; j++) 
		{
            soma += A[i][j] * x[j];
        }
        y[i] = soma;
    }
    gettimeofday(&end, NULL);
    printf("Quantidade de Colunas: %d\n", colunas);
    printf("Tamanho do vetor y: %lu bytes\n", linhas * sizeof(int));
    printf("Tamanho da matriz A: %lu bytes\n", linhas * sizeof(int*) + linhas * colunas * sizeof(int));
    printf("Tempo de execução (acesso por linhas): %.2f ms\n", timeFormater());
}

void multiplicacaoPorColunas(int linhas, int colunas, int **A, int *x, int *y)
{
    printf("MultiplicacaoPorColunas\n");
    for (int i = 0; i < linhas; i++)
        y[i] = 0;

    gettimeofday(&start, NULL);
    for (int j = 0; j < colunas; j++) 
	{
        for (int i = 0; i < linhas; i++) 
		{
            y[i] += A[i][j] * x[j];
        }
    }
    gettimeofday(&end, NULL);
    printf("Quantidade de Colunas: %d\n", colunas);
    printf("Tamanho do vetor y: %lu bytes\n", linhas * sizeof(int));
    printf("Tamanho da matriz A: %lu bytes\n", linhas * sizeof(int*) + linhas * colunas * sizeof(int));
    printf("Tempo de execução (acesso por colunas): %.2f ms\n", timeFormater());
}

int main(void)
{
    const int numeroDeTestes[4] =
    {10, 100, 1000, 10000};
    for (int k = 0; k < 4; k++)
    {
        int colunas = numeroDeTestes[k];
        int linhas = numeroDeTestes[k];

        int *x = malloc(colunas * sizeof(int));
        int *y_linhas = malloc(linhas * sizeof(int));
        int *y_colunas = malloc(linhas * sizeof(int));

        int **A = malloc(linhas * sizeof(int*));

        for (int i = 0; i < linhas; i++)
            A[i] = malloc(colunas * sizeof(int));

        for (int i = 0; i < colunas; i++)
            x[i] = 1;

        for (int i = 0; i < linhas; i++)
            for (int j = 0; j < colunas; j++)
                A[i][j] = 1;

        multiplicacaoPorLinhas(linhas, colunas, A, x, y_linhas);

        printf("------------------------------\n");

        multiplicacaoPorColunas(linhas, colunas, A, x, y_colunas);

        printf("==============================\n");

        for (int i = 0; i < linhas; i++)
            free(A[i]);

        free(A);
        free(x);
        free(y_linhas);
        free(y_colunas);
    }

    return 0;
}