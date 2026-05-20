#!/bin/bash 
#SBATCH --job-name=primos_master_worker
#SBATCH --time=0-0:15
#SBATCH --partition=intel-128
#SBATCH --ntasks=8
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1

make clean
make

MAX_PRIME=10000000

echo "========================================================"
echo "=== Testando MPI: Escalonador Dinamico (Lider-Trabalhador)"
echo "=== Calculo de primos ate $MAX_PRIME"
echo "========================================================"

echo "--------------------------------------------------------"
echo "1. Baseline Sequencial"
echo "--------------------------------------------------------"
mpirun -np 1 ./primos_master_worker $MAX_PRIME 100000

echo "--------------------------------------------------------"
echo "2. Variando Quantidade de Trabalhadores (Chunk Fixo 100K)"
echo "--------------------------------------------------------"
# np 2 = 1 Lider, 1 Trabalhador
mpirun -np 2 ./primos_master_worker $MAX_PRIME 100000
# np 4 = 1 Lider, 3 Trabalhadores
mpirun -np 4 ./primos_master_worker $MAX_PRIME 100000
# np 8 = 1 Lider, 7 Trabalhadores
mpirun -np 8 ./primos_master_worker $MAX_PRIME 100000

echo "--------------------------------------------------------"
echo "3. Variando Quantidade de Tarefas (Workers Fixos np=8)"
echo "--------------------------------------------------------"
# Poucas Tarefas (Chunks Grandes - 1M)
mpirun -np 8 ./primos_master_worker $MAX_PRIME 1000000
# Muitas Tarefas (Chunks Pequenos - 10k)
mpirun -np 8 ./primos_master_worker $MAX_PRIME 10000