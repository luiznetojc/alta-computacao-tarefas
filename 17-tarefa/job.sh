#!/bin/bash
#SBATCH --job-name=matvec_tarefa17
#SBATCH --time=0-0:30
#SBATCH --partition=intel-128
#SBATCH --mem=64G
#SBATCH --ntasks=4
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1

make clean
make

echo "Processos,Dimensao,Tempo" > resultados.csv

DIMENSOES=(1200 2400 4800 11520 16000 35360)
PROCESSOS=(1 2 4)

for p in "${PROCESSOS[@]}"; do
    for dim in "${DIMENSOES[@]}"; do
        # Dimensao eh multipla de Processos (OK para todos os valores e processos listados)
        res=$(mpirun -np $p ./matvec $dim $dim)
        echo "$p,$dim,$res" >> resultados.csv
    done
done
