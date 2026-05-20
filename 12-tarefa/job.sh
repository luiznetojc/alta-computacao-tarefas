#!/bin/bash 
#SBATCH --job-name=navier_escala 
#SBATCH --time=0-0:30
#SBATCH --partition=intel-128
#SBATCH --cpus-per-task=16
#SBATCH --hint=compute_bound

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

# Compila os códigos
make clean
make

# Script para avaliar a escalabilidade
echo "======================================"
echo "Iniciando benchmarks (Escalabilidade)"
echo "Número máximo de threads do Job: $SLURM_CPUS_PER_TASK"
echo "======================================"

echo -e "\n=== Teste navier_serial ==="
./navier_serial

for THREADS in 1 2 4 8 16; do
    echo -e "\n======================="
    echo "Testando com $THREADS Threads"
    echo "======================="

    echo "--- navier_static ---"
    ./navier_static $THREADS

    echo "--- navier_static_collapse ---"
    ./navier_static_collapse $THREADS

    echo "--- navier_dynamic ---"
    ./navier_dynamic $THREADS

    echo "--- navier_guided ---"
    ./navier_guided $THREADS
done
