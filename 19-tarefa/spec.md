# Especificação da Tarefa 19: Adição de Vetores com OpenMP Offload (GPUs)

Baseado na análise dos arquivos no diretório `openmp-tutorial-tmp/Solutions/`, elaboramos as diretrizes para a implementação de diferentes versões do programa de adição de vetores (`vadd.c`) utilizando OpenMP. O objetivo é contemplar as otimizações e variações explicadas nos slides 27 e 48 do tutorial de OpenMP.

## 1. Versões do Código C

O implementador deve criar três arquivos fonte baseados no código padrão de `vadd.c` / `vadd_par.c`. O código original já contém as medições de tempo (`omp_get_wtime()`) e verificação de corretude. Cada versão ajustará os pragmas no loop de adição de vetores.

### 1.1 `vadd_cpu.c` (Baseline CPU)
**Objetivo:** Execução exclusiva na CPU com paralelismo multithread para servir de baseline de desempenho.
**Diretriz de Implementação:**
- Utilize a base do arquivo `vadd_par.c`.
- Certifique-se de que o pragma `#pragma omp parallel for` seja utilizado no loop principal que soma os vetores (`c[i] = a[i] + b[i]`).
- Garanta que não haja nenhuma diretiva de `target` ou offload nesta versão.

### 1.2 `vadd_slide27.c` (Abordagem Básica - Slide 27)
**Objetivo:** Introduzir o offloading básico para a GPU com a menor intervenção possível. Esta abordagem deixa grande parte das decisões de paralelismo e movimentação de dados para a implementação/compilador.
**Diretriz de Implementação:**
- Utilize a base do arquivo `vadd_target.c`.
- O loop de adição deve ser precedido por pragmas básicos de target:
  ```c
  #pragma omp target
  #pragma omp loop
  for (int i=0; i<N; i++) {
      c[i] = a[i] + b[i];
  }
  ```
- O compilador inferirá o mapeamento dos arrays (`a`, `b`, `c`) implicitamente. Esta é a forma mais simples (apresentada no slide 27) para transferir a execução de um loop para o device (GPU).

### 1.3 `vadd_slide48.c` (Abordagem Otimizada - Slide 48)
**Objetivo:** Explorar o paralelismo hierárquico da GPU (times de threads e execução paralela dentro delas) e explicitar o mapeamento de memória, resultando em maior performance.
**Diretriz de Implementação:**
- O loop de adição deve ser precedido pelos pragmas que ativam explicitamente times, distribuição e mapeamento:
  ```c
  #pragma omp target teams distribute parallel for map(to: a[0:N], b[0:N]) map(from: c[0:N])
  for (int i=0; i<N; i++) {
      c[i] = a[i] + b[i];
  }
  ```
- O uso de `teams distribute parallel for` reflete a estrutura avançada apresentada no slide 48 do tutorial, dividindo o loop entre diferentes "teams" na GPU e distribuindo as iterações entre as threads de cada team.
- As cláusulas de mapeamento explícito (`map(to:)`, `map(from:)`) previnem cópias de dados desnecessárias e otimizam a comunicação CPU/GPU.

---

## 2. Estrutura do `Makefile`

Seguindo os padrões adotados na `18-tarefa`, o implementador deve criar um `Makefile` com as seguintes características:
- **Variáveis:** 
  - Definir `CC` para o compilador apropriado com suporte a OpenMP e offloading (ex: `gcc`, `clang`, `nvc`).
  - Definir `CFLAGS` contendo as flags de otimização e ativação do OpenMP offload (ex: `-O3 -fopenmp` ou equivalente dependendo do compilador e do cluster).
- **Targets:**
  - `vadd_cpu`
  - `vadd_slide27`
  - `vadd_slide48`
- **Regras:**
  - `all: vadd_cpu vadd_slide27 vadd_slide48`
  - Cada binário deve ser compilado a partir de seu respectivo arquivo `.c` usando `$(CC) $(CFLAGS)`.
  - Regra `clean` que remove os arquivos executáveis gerados.

## 3. Estrutura do `job.sh`

Para a submissão e execução no cluster via SLURM, o `job.sh` deve ser estruturado de forma análoga à `18-tarefa`:
- **Diretivas SLURM (`#SBATCH`):**
  - Configurar nome do job (ex: `vadd_omp_gpu`), limites de tempo e memória.
  - **Fundamental:** Garantir que o script requisita uma partição (ou nó) de GPU válida (ex: `#SBATCH --partition=gpu`), já que o principal objetivo é testar o OpenMP offload.
- **Fluxo de Execução:**
  1. Compilar os executáveis (`make clean && make`).
  2. Executar de forma estruturada as três versões de modo a comparar os seus tempos de execução:
     ```bash
     echo "=== Executando vadd_cpu (Baseline) ==="
     ./vadd_cpu
     
     echo "=== Executando vadd_slide27 (Target Basico) ==="
     ./vadd_slide27
     
     echo "=== Executando vadd_slide48 (Target Otimizado) ==="
     ./vadd_slide48
     ```
  3. *(Opcional)* Se desejável avaliar métricas além da execução padrão (ex. escalar tamanho da carga de dados `N`), adicionar o loop de tamanhos análogo ao que foi feito com `$DIMENSOES` na `18-tarefa`.
