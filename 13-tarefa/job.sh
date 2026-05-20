#!/bin/bash 
#SBATCH --job-name=navier_afinidade 
#SBATCH --time=0-1:00
#SBATCH --partition=intel-128
#SBATCH --nodes=1
#SBATCH --cpus-per-task=16
#SBATCH --hint=compute_bound

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

# Compila os códigos
make clean
make

echo "======================================"
echo "Avaliação de Afinidade (Task 13)"
echo "Número máximo de threads do Job: $SLURM_CPUS_PER_TASK"
echo "======================================"

# Vamos testar o código 'navier_static' ou o que apresentou melhor tempo na tarefa 12.
# Aqui usamos navier_static como referência.
EXEC="./navier_static"

# Definimos as combinações de afinidade do OpenMP
PLACES=("cores" "sockets" "threads")
BINDS=("close" "spread")

# Vamos avaliar para 4, 8 e 16 threads
for THREADS in 4 8 16; do
    export OMP_NUM_THREADS=$THREADS
    echo -e "\n============================================="
    echo "Testando com $THREADS Threads"
    echo "============================================="

    # Sem afinidade explícita (padrão do sistema)
    export OMP_PROC_BIND=false
    export OMP_PLACES=
    echo "--- Afinidade PADRÃO (Sem binding explícito) ---"
    $EXEC $THREADS

    # Testando combinações de OMP_PLACES e OMP_PROC_BIND
    for PLACE in "${PLACES[@]}"; do
        for BIND in "${BINDS[@]}"; do
            export OMP_PLACES=$PLACE
            export OMP_PROC_BIND=$BIND
            echo "--- Afinidade: PLACES=$PLACE, BIND=$BIND ---"
            $EXEC $THREADS
        done
    done
done
