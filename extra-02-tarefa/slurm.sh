#!/bin/bash
#SBATCH --job-name=derivative_cuda
#SBATCH --output=resultado.txt
#SBATCH --partition=gpu-8-v100
#SBATCH --gres=gpu:1
#SBATCH --time=00:05:00

# Load CUDA module if needed (often necessary on clusters)
module load compilers/nvidia/cuda/12.6

# Compile the code
nvcc -O3 tarefa.cu -o tarefa

# Run the executable
./tarefa
