#!/bin/bash 
#SBATCH --job-name=heat_diffusion
#SBATCH --time=0-0:15
#SBATCH --partition=intel-128
#SBATCH --ntasks=8
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1

# O módulo do MPI precisa estar carregado no NPAD
# module load mpi/openmpi-x86_64

make clean
make

echo "================================================="
echo "=== Testando MPI c/ 8 Processos MPI"
echo "================================================="

echo -e "\n1. Versao Blocante (MPI_Send/Recv)"
mpirun -np 8 ./heat_blocking

echo -e "\n2. Versao Nao-Blocante (MPI_Isend/Irecv + Wait puro)"
mpirun -np 8 ./heat_nonblocking

echo -e "\n3. Versao Otimizada c/ Sobreposicao (Isend/Irecv + Processamento Interno Hibrido)"
mpirun -np 8 ./heat_overlap
