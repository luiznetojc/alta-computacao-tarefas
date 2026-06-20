#!/bin/bash
#SBATCH --job-name=heat_omp_gpu
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
# Add nsys to path
export PATH=/opt/npad/shared/compilers/nvidia_hpc_sdk/24.11/Linux_x86_64/24.11/profilers/Nsight_Systems/bin:$PATH

make clean
make

echo "=== Executando heat_cpu (N=1000, 100 steps) ==="
./heat_cpu 1000 100

echo "=== Executando heat_gpu_basic (N=1000, 100 steps) ==="
./heat_gpu_basic 1000 100

echo "=== Executando heat_gpu_opt (N=1000, 100 steps) ==="
./heat_gpu_opt 1000 100

echo "=== Profiling heat_gpu_opt com nsys (N=1000, 100 steps) ==="
nsys profile --stats=true --force-overwrite=true -o heat_1000 ./heat_gpu_opt 1000 100

echo ""
echo "=== Executando heat_cpu (N=4000, 100 steps) ==="
./heat_cpu 4000 100

echo "=== Executando heat_gpu_basic (N=4000, 100 steps) ==="
./heat_gpu_basic 4000 100

echo "=== Executando heat_gpu_opt (N=4000, 100 steps) ==="
./heat_gpu_opt 4000 100

echo "=== Profiling heat_gpu_opt com nsys (N=4000, 100 steps) ==="
nsys profile --stats=true --force-overwrite=true -o heat_4000 ./heat_gpu_opt 4000 100
