#!/bin/bash
#SBATCH --job-name=matvec_tarefa18
#SBATCH --time=0-0:45
#SBATCH --partition=intel-128
#SBATCH --mem=64G
#SBATCH --ntasks=4
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1

make clean
make

cd ../17-tarefa
make clean
make
cd ../18-tarefa

echo "Versao,Processos,Dimensao,Tempo" > resultados.csv

DIMENSOES=(1200 2400 4800 11520 16000 35360)
PROCESSOS=(1 2 4)

for p in "${PROCESSOS[@]}"; do
    for dim in "${DIMENSOES[@]}"; do
        
        # Tarefa 17 (Distribuicao por Linhas)
        res_row=$(mpirun -np $p ../17-tarefa/matvec $dim $dim)
        echo "row,$p,$dim,$res_row" >> resultados.csv

        # Tarefa 18 (Distribuicao por Colunas - Vector Only)
        # res_col_vec=$(mpirun -np $p ./matvec_col_vector $dim $dim)
        # echo "col_vector,$p,$dim,$res_col_vec" >> resultados.csv

        # Tarefa 18 (Distribuicao por Colunas - Resized)
        res_col_res=$(mpirun -np $p ./matvec_col_resized $dim $dim)
        echo "col_resized,$p,$dim,$res_col_res" >> resultados.csv

    done
done
