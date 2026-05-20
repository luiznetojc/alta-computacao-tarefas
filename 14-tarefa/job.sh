#!/bin/bash 
#SBATCH --job-name=pingpong_mpi
#SBATCH --time=0-0:10
#SBATCH --partition=intel-128
#SBATCH --ntasks=2
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1

# O módulo do MPI precisa estar carregado no NPAD
# module load mpi/openmpi-x86_64

make clean
make

echo "=== MPI_Send ==="
mpirun -np 2 ./pingpong_send

echo "================="
echo "=== MPI_Ssend ==="
mpirun -np 2 ./pingpong_ssend

echo "================="
echo "=== MPI_Bsend ==="
mpirun -np 2 ./pingpong_bsend

echo "================="
echo "=== MPI_Rsend ==="
mpirun -np 2 ./pingpong_rsend
