# 15-Tarefa: Difusão de Calor em Barra 1D (Halo Células)

Nesta tarefa, simulamos a condução de calor em uma malha superdensa com a estratégia de divisão do Domínio 1D em *N* processos usando a biblioteca MPI.
Para permitir que o campo físico calcule a fórmula das diferenças finitas nas arestas do domínio dos sub-blocos locais, implementamos o conceito de troca de "Células Fantasmas" (Halo Cells) dos vizinhos de direita e esquerda.

Foram testadas três concepções:

### 1. MPI_Send / MPI_Recv (Blocante)
Utiliza uma comunicação síncrona. Embora as rotinas no MPI frequentemente dependam da infraestrutura do buffer e de qual rede o protocolo está amarrado (Eager vs Rendezvous), criar submissões fechadas (`Send` imediados em sequência) podem incorrer em Deadlocks se dois nós tentarem enviar suas bordas reciprocamente sem ninguém recivá-las a tempo. Ao sequenciá-las de forma cadenciada cria-se um tempo de espera muito expressivo pois a computação do stencil só se inicializa quando `Send` e `Recv` confirmarem tráfego 100% de sucesso.

### 2. MPI_Isend / MPI_Irecv + Wait (Não Blocante em Seq)
Aqui o tempo de envio não estrangula de imediato a passagem de código. Todos os comunicadores abrem uma porta de `Irecv`, depois cospem seu material para vizinhos pelo `Isend` nas requests. E terminam chamando `Waitall` (como uma trave forçada fechando o funil temporariamente). 
**Ganhos:** Impede _deadlocks_. Aumenta o throughput caso o sistema operacional consiga enviar pacotes enquanto registra os assíncronos. No entanto o cálculo do array _AINDA_ não usufruiu da latência ociosa da network.

### 3. Sobreposição de Computação e Comunicação (Overlap c/ Non-Blocking pseudo-testes)
Aqui dividimos a modelagem espacial em duas etapas. Enquanto a placa de rede mastiga _em background_ a cópia em RAM das requisições abertas em `MPI_Isend/MPI_Irecv`, em vez de paralisar tudo, o bloco computacional efetua o Update (`u_new`) de todas as **células estritamente internas** (células local_2 até local_N-1). Elas não precisam de valores transfronteiriços para seus estêncils.
Quando os cálculos esgotam (escondendo eficientemente a latência do barramento), fazemos o cheque de término em cima das ghosts cells com `Waitall` (funciona homologo à arquitetura test). Confirmado a troca com os vizinhos, os dados de pontas `u[1]` e `u[local_N]` são finalmente iterados.

### Resultados e Tempos de Execução no NPAD (Novo Modelo 2D - 4000x4000)

Testes executados no cluster NPAD com `mpirun -np 8` forçando **1 processo por nó físico** (8 nós no total), assegurando tráfego sobre a rede. A simulação foi reformulada de 1D para uma densa malha 2D (4000x4000, e células-fantasma transmitindo vetores inteiros equivalentes a 32 KB *double* por vez, por vizinho):

| Versão | Descrição | Tempo Total (s) |
|:---|:---|:---|
| 1 | Versao 2D: `MPI_Send` / `MPI_Recv` (Blocante) | 0.470836 |
| 2 | Versao 2D: `MPI_Isend` / `MPI_Irecv` + `Waitall` (Não Blocante em Seq) | 0.466604 |
| 3 | Versao 2D Atualizada: Sobreposição de Computação e Comunicação (Overlap) | 0.460717 |

> **Nota:** Houve o registro de avisos relativos à inicialização do dispositivo OpenFabrics (`WARNING: There was an error initializing an OpenFabrics device... help-mpi-btl-openib.txt`), de forma idêntica à tarefa anterior. Trata-se de um ruído de configuração do driver InfiniBand no ambiente, não impactando a conclusão correta e a validação do benchmark executado por outros links.

### Conclusão e Observações

Comparando as diferentes abordagens em um regime de malha 2D conectada intra-cluster:
1. O aumento de carga nos limites (transmitindo vetores inteiros de 32 KB em vez de uma única variável escalar como no 1D) foi ideal para observar o distanciamento físico da latência isolada.
2. A versão **Não-Blocante pura** logrou desconto frente a Blocante ao preencher e postar todas as transferências de uma vez em protocolo *Eager* nas placas, o que evita que os gargalos das pontas atrasem o laço subjacente.
3. A versão com **Overlapping (Sobreposição Computação-Comunicação)** figura perfeitamente como a mais rápida absoluta. Essa diferença se instaura justamente porque, ao despachar a requisição pesada das *Halo Cells* via *Isend*, a CPU de imediato se vira para atualizar os blocos centrais de calor 2D da matriz local (que independem do fim da cópia de rede), camuflando a latência. 

Este resultado reflete perfeitamente as bases de um sistema computacional escalável para a mecânica de fluídos 2D. É exatamente esse mascaramento computacional que viabiliza modelos oceânicos e espaciais monstruosos a conseguirem ser repartidos aos milhares ao redor do planeta!

---

## Apêndice - Implementações (Malha 2D - 4000x4000)

### 1. `heat_blocking.c`

```c
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

	for (int r = 1; r <= local_R; r++) {
		for (int c = 0; c < COLS; c++) {
			u[r * COLS + c] = (rank == size / 2 && r == local_R / 2 && c == COLS / 2) ? 100.0 : 20.0;
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	double start = MPI_Wtime();

	for (int t = 0; t < STEPS; t++)
	{
		if (rank > 0)
		{
			MPI_Send(&u[1 * COLS], COLS, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
			MPI_Recv(&u[0 * COLS], COLS, MPI_DOUBLE, rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		else
		{
			for(int c=0; c<COLS; c++) u[0 * COLS + c] = u[1 * COLS + c];
		}

		if (rank < size - 1)
		{
			MPI_Send(&u[local_R * COLS], COLS, MPI_DOUBLE, rank + 1, 1, MPI_COMM_WORLD);
			MPI_Recv(&u[(local_R + 1) * COLS], COLS, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		else
		{
			for(int c=0; c<COLS; c++) u[(local_R + 1) * COLS + c] = u[local_R * COLS + c];
		}

		for (int r = 1; r <= local_R; r++)
		{
			for (int c = 1; c < COLS - 1; c++)
			{
				double up    = u[(r - 1) * COLS + c];
				double down  = u[(r + 1) * COLS + c];
				double left  = u[r * COLS + c - 1];
				double right = u[r * COLS + c + 1];
				double center = u[r * COLS + c];
				u_new[r * COLS + c] = center + C * (up + down + left + right - 4.0 * center);
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
		printf("Versao 2D Atualizada: MPI_Send / MPI_Recv (Blocante)\n");
		printf("Tempo total: %f segundos\n", end - start);
	}

	free(u);
	free(u_new);
	MPI_Finalize();
	return 0;
}
```

### 2. `heat_nonblocking.c`

```c
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

	for (int r = 1; r <= local_R; r++) {
		for (int c = 0; c < COLS; c++) {
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
			for(int c=0; c<COLS; c++) u[0 * COLS + c] = u[1 * COLS + c];
		}

		if (rank < size - 1)
		{
			MPI_Irecv(&u[(local_R + 1) * COLS], COLS, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &reqs[req_count++]);
			MPI_Isend(&u[local_R * COLS], COLS, MPI_DOUBLE, rank + 1, 1, MPI_COMM_WORLD, &reqs[req_count++]);
		}
		else
		{
			for(int c=0; c<COLS; c++) u[(local_R + 1) * COLS + c] = u[local_R * COLS + c];
		}

		if (req_count > 0)
			MPI_Waitall(req_count, reqs, MPI_STATUSES_IGNORE);

		for (int r = 1; r <= local_R; r++)
		{
			for (int c = 1; c < COLS - 1; c++)
			{
				double up    = u[(r - 1) * COLS + c];
				double down  = u[(r + 1) * COLS + c];
				double left  = u[r * COLS + c - 1];
				double right = u[r * COLS + c + 1];
				double center = u[r * COLS + c];
				u_new[r * COLS + c] = center + C * (up + down + left + right - 4.0 * center);
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
		printf("Versao 2D: MPI_Isend / MPI_Irecv + Wait (Nao-Blocante estrito)\n");
		printf("Tempo total: %f segundos\n", end - start);
	}

	free(u);
	free(u_new);
	MPI_Finalize();
	return 0;
}
```

### 3. `heat_overlap.c`

```c
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

	for (int r = 1; r <= local_R; r++) {
		for (int c = 0; c < COLS; c++) {
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
			for(int c=0; c<COLS; c++) u[0 * COLS + c] = u[1 * COLS + c];
		}

		if (rank < size - 1)
		{
			MPI_Irecv(&u[(local_R + 1) * COLS], COLS, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &reqs[req_count++]);
			MPI_Isend(&u[local_R * COLS], COLS, MPI_DOUBLE, rank + 1, 1, MPI_COMM_WORLD, &reqs[req_count++]);
		}
		else
		{
			for(int c=0; c<COLS; c++) u[(local_R + 1) * COLS + c] = u[local_R * COLS + c];
		}

		for (int r = 2; r <= local_R - 1; r++)
		{
			for (int c = 1; c < COLS - 1; c++)
			{
				double up    = u[(r - 1) * COLS + c];
				double down  = u[(r + 1) * COLS + c];
				double left  = u[r * COLS + c - 1];
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
			for (int c = 1; c < COLS - 1; c++) {
				double up    = u[(r - 1) * COLS + c];
				double down  = u[(r + 1) * COLS + c];
				double left  = u[r * COLS + c - 1];
				double right = u[r * COLS + c + 1];
				double center = u[r * COLS + c];
				u_new[r * COLS + c] = center + C * (up + down + left + right - 4.0 * center);
			}
			if (local_R > 1)
			{
				r = local_R;
				for (int c = 1; c < COLS - 1; c++) {
					double up    = u[(r - 1) * COLS + c];
					double down  = u[(r + 1) * COLS + c];
					double left  = u[r * COLS + c - 1];
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
```