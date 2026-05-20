# 13-Tarefa: Avaliação de Afinidade

## Metodologia
Nesta tarefa, testamos o impacto de diferentes afinidades de *threads* geradas pelo OpenMP no desempenho do algoritmo solver de Navier-Stokes. Utilizamos um nó dedicado da partição de `cluster` no NPAD com 16 CPUs dedicadas. 

Afinidade de *threads* diz respeito à forma como as threads de execução estão amarradas (bind) à hierarquia física do processador (Cores, Sockets, Hardware Threads).

As diretivas avaliadas foram:
* **OMP_PLACES**: `cores`, `sockets`, `threads`
* **OMP_PROC_BIND**: `close` (threads adjacentes entre si) e `spread` (espalhadas para distribuição equilibrada nas caches e controle térmico).

## Resultados Obtidos

**(Preencher com os tempos retornados do job no NPAD)**

| Threads | Afinidade (Places/Bind) | Tempo (s) | Speedup (T1/Tn) |
|---------|-------------------------|-----------|-----------------|
| 1       | Padrão                  | -         | 1.0x            |
| 16      | Padrão                  | -         | -               |
| 16      | cores / close           | -         | -               |
| 16      | cores / spread          | -         | -               |
| 16      | sockets / close         | -         | -               |
| 16      | sockets / spread        | -         | -               |
| 16      | threads / close         | -         | -               |
| 16      | threads / spread        | -         | -               |

## Discussão
**(Preencher a análise)**

* **close vs spread:** Avalie qual estratégia lidou melhor com a lei de recursos locais vs cache compartilhada. Muitas vezes `spread` distribui melhor a contenção térmica e a banda de memória, enquanto `close` melhora cenários que precisam de altíssimo reuso da L1/L2.
* **cores vs threads:** Observe como forçar a execução isolada em *cores* físicos diferentes evita a superlotação do *Hyper-Threading*. 

O gargalo de escalabilidade que antes parecia ser de sincronização pôde (ou não) ser minimizado escolhendo o `OMP_PROC_BIND=spread` e `OMP_PLACES=cores`.