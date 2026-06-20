#!/bin/bash
#SBATCH --job-name=vadd_omp_gpu
#SBATCH --time=0-0:15
#SBATCH --partition=gpu-8-v100
#SBATCH --gres=gpu:1
#SBATCH --mem=16G
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --cpus-per-task=4

# Load NVIDIA HPC SDK for nvc compiler and OpenMP offload support
export PATH=/opt/npad/shared/compilers/nvidia_hpc_sdk/24.11/Linux_x86_64/24.11/compilers/bin:$PATH
export LD_LIBRARY_PATH=/opt/npad/shared/compilers/nvidia_hpc_sdk/24.11/Linux_x86_64/24.11/compilers/lib:$LD_LIBRARY_PATH

make clean
make

echo "=== Executando vadd_cpu (Baseline) ==="
./vadd_cpu

echo "=== Executando vadd_slide27 (Target Basico) ==="
./vadd_slide27

echo "=== Executando vadd_slide48 (Target Otimizado) ==="
./vadd_slide48
