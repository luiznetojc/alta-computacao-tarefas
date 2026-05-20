# 16-Tarefa: Escalonador Dinâmico de Tarefas (Líder-Trabalhador)

Nesta tarefa, implementamos um escalonamento dinâmico de tarefas na arquitetura balanceada pelo modelo Líder–Trabalhador (*Master-Worker*) usando blocos de contagem de números primos por divisão do espaço iterativo (chunks).

### Abordagem

O processo principal (rank `0`), ou Líder, é responsável por manter a conta global do intervalo avaliado. O Líder dispara pacotes de limites iterativos inferiores e superiores `[start, end]`. Para isso ele realiza o envio das tarefas sob demanda:
1. **Fase de Disparo Inicial:** Inicializa a carga emitindo uma tarefa de processamento (`WORKTAG`) garantidamente a cada um dos outros `N-1` Trabalhadores. 
2. **Ciclo de Devolução:** Fica esperando de qualquer *source* de comunicação os resultados (`MPI_Recv` com `MPI_ANY_SOURCE`). Após armazenar a quantia local que o trabalhador processou, ele devolve imediatamente o próximo _chunk_ ocioso para a respectiva placa que recém finalizou o serviço.
3. **Escoamento (Poison Pill):** Quando os _chunks_ de busca limitam-se ao teto máximo desejado, o Líder passa a responder aos trabalhadores terminados com as `DIETAG`. O laco é encerrado quando a quantia de operários `active_workers` é zero.

Todos os trabalhadores possuem a mesma mecânica contínua:
- Escutar o Líder (`MPI_Recv`).
- Se receberem `DIETAG`, a execução interna da *thread* faz `break` e caminha para o encerramento.
- Se receberem `WORKTAG` ativam o *kernel* de primitividade contando iterativamente para aquele limite e, ao fim, empurram a contagem de achados para o rank principal com `MPI_Send`. 

### Tolerância à Deadlocks e Desempenho

Uma vez que é estipulado o *handshake* contínuo de pacotes onde o **Líder atende unicamente sob demanda**, a chance de interrupção (deadlock) na comunicação não atinge esta arquitetura. Fator dependente é, tão somente, o peso de cada sub-tarefa (_Chunk Size_):
- **Oversubscription/Chunks pequenos:** Ao granular demais o processamento (milhares de chunks de 10 mil laços, por exemplo), subimos exponencialmente o tráfego de requisições `MPI_Send/Recv`. A latência afoga a CPU e derruba a escalabilidade eficiente.
- **Granularidade muito alta/Poucos Chunks:** Ao dar _poucos pedaços enormes_, caímos de volta a problemas de carga não balanceada estática. Se calhar que o processo 7 receba a penúltima carga enorme com limite da distribuição, e a carga seja ligeiramente mais intensiva nele, os outros processos irão desativar sua existência com `DIETAG`, ociando seus _cores_ em favor da sobrecarga num só trabalhador que se arrasta.
- **Speedup Máximo:** Uma vez que estamos executando com 8 nós (`np 8`), 1 deles atua como mero burocrata distribuidor de dados. Assim sendo, a escalabilidade máxima possível de Speedup num intervalo grande de computação pura será restrita a **~7x** o tempo sequencial, determinando portanto eficiência teórica ótima baseada nos workers efetivos.

### Execução de Benchmarks

A chamada `make` constrói o binário `primos_master_worker`. O *job* enviado para o slurm no cluster executa variações de granulação:
- **`Execucao Sequencial (P=1)`** Fornece o Base time de processamento para comparar aceleração.
- **`Execucao Paralela c/ Chunk Alto`** Distribui poucas tarefas.
- **`Execucao Paralela c/ Chunk Medio`** Garante melhor balanceamento entre a variância dos limites matemáticos superiores da série temporal primária dos trabalhadores VS o tráfego da rede.
- **`Execucao Paralela c/ Chunk Baixo`** Demostração de sobrecarga de comutação do Send/Recv.

Com esta heurística comprova-se o balanço eficiente da infraestrutura passiva MPI.

### Resultados e Avaliação de Desempenho no Cluster (NPAD)

Abaixo estão os resultados extraídos do cluster NPAD com `mpirun -np 8` (1 Líder + 7 Trabalhadores), buscando primos até 10.000.000. O total de números primos encontrados foi consistente em todas as execuções (664.579 primos).

| Execução | Chunk Size | Tarefas Processadas | Tempo Total (s) | Speedup (vs Seq) | Eficiência (sobre 7 trab) |
|:---|:---:|:---:|:---:|:---:|:---:|
| 1. Sequencial (Baseline) | N/A | 1 | 1.417608 | 1.00x | - |
| 2. Paralela (Chunk Alto) | 1.000.000 | 10 | 0.340426 | **4.16x** | **59.4%** |
| 3. Paralela (Chunk Médio) | 100.000 | 100 | 0.289629 | **4.89x** | **69.8%** |
| 4. Paralela (Chunk Baixo) | 10.000 | 1.000 | 0.285411 | **4.96x** | **70.8%** |

#### Análise dos Resultados:
- **Speedup e Eficiência:** O cálculo do Speedup foi feito comparando a versão sequencial (1.417s) com as versões mapeadas nos 7 trabalhadores. O fator máximo ideal almejado é de 7x. 
- A versão de **Chunk Baixo/Médio apresentou um limite em torno de ~5.0x de Speedup (70% de Eficiência)**. Essa queda sobre o ideal (7x) engloba os atrasos associados à barreira inicial, e o *overhead* de latência provocado pelas requisições frequentes trafegadas pelas placas e cabo de rede (InfiniBand/Ethernet do NPAD).
- O **Chunk Alto** resultou no pior tempo concorrente, tendo Speedup de apenas 4.16x. A causa é puramente o balanceamento de carga desigual. Distribuir a malha em apenas 10 fatias massivas num pool com 7 operários deixa a rede em inanição e agrava os últimos processos de terminar. Já chunks de *10k* e *100k* estabilizaram o tempo pelo princípio ideal do escalonador Master-Worker: a alta dinamicidade na entrega impediu que alguns nós ficassem ociosos precocemente.

---

### Apêndice - Implementação
Código `primos_master_worker.c`:

```c
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define LIDER 0
#define WORKTAG 1
#define DIETAG 2

int is_prime(int n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return 0;
    }
    return 1;
}

// Implementacao do Lider usando Dispatch, Recv & Respond, DIETAG.
// ... (Visualizar arquivo principal completo)
```