#!/bin/bash
#SBATCH --job-name=heat_gpu_perf
#SBATCH --partition=gpu-8-v100
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --gpus=1
#SBATCH --time=00:10:00
#SBATCH --output=slurm-%j.out

# Workaround for module loading issue
export PATH=/opt/npad/shared/compilers/nvidia_hpc_sdk/24.11/Linux_x86_64/24.11/compilers/bin:$PATH

echo "Compiling..."
make clean
make

echo "Running tests and generating CSV data..."

# Create CSV header
echo "Version,Size,SolveTime_s,TotalTime_s" > results.csv

SIZES="500 1000 2000 4000 8000"
NSTEPS=100

for s in $SIZES; do
    echo "=== Running CPU Size: $s ==="
    OUTPUT=$(./heat_cpu $s $NSTEPS)
    SOLVE=$(echo "$OUTPUT" | grep "Solve time (s):" | awk '{print $4}')
    TOTAL=$(echo "$OUTPUT" | grep "Total time (s):" | awk '{print $4}')
    echo "CPU,$s,$SOLVE,$TOTAL" >> results.csv

    echo "=== Running GPU Basic Size: $s ==="
    OUTPUT=$(./heat_gpu_basic $s $NSTEPS)
    SOLVE=$(echo "$OUTPUT" | grep "Solve time (s):" | awk '{print $4}')
    TOTAL=$(echo "$OUTPUT" | grep "Total time (s):" | awk '{print $4}')
    echo "GPU_Basic,$s,$SOLVE,$TOTAL" >> results.csv

    echo "=== Running GPU Opt Size: $s ==="
    OUTPUT=$(./heat_gpu_opt $s $NSTEPS)
    SOLVE=$(echo "$OUTPUT" | grep "Solve time (s):" | awk '{print $4}')
    TOTAL=$(echo "$OUTPUT" | grep "Total time (s):" | awk '{print $4}')
    echo "GPU_Opt,$s,$SOLVE,$TOTAL" >> results.csv
done

echo "Profiling with nsys for n=1000 nsteps=10"
nsys profile -o profile_basic --stats=true ./heat_gpu_basic 1000 10 > nsys_basic.txt
nsys profile -o profile_opt --stats=true ./heat_gpu_opt 1000 10 > nsys_opt.txt

echo "Done."
