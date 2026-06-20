#!/bin/bash
# Script para interagir com o NPAD para transferência de arquivos e execução de comandos
# Uso: 
#   ./npad_skill.sh transfer <diretorio_local> [diretorio_remoto]
#   ./npad_skill.sh execute <diretorio_remoto> <script_sbatch>

ACTION=$1
LOCAL_DIR=$2
REMOTE_DIR=${3:-"~"}
REMOTE_HOST="super-pc"

if [ "$ACTION" == "transfer" ]; then
    echo "Transferindo $LOCAL_DIR para $REMOTE_HOST:$REMOTE_DIR..."
    scp -r "$LOCAL_DIR" "$REMOTE_HOST:$REMOTE_DIR"
    if [ $? -eq 0 ]; then
        echo "Transferência concluída com sucesso."
    else
        echo "Erro na transferência."
        exit 1
    fi
elif [ "$ACTION" == "execute" ]; then
    echo "Acessando $REMOTE_HOST e executando o job $LOCAL_DIR..."
    # No caso execute, LOCAL_DIR será tratado como o diretório remoto do projeto e $3 como o script
    WORK_DIR=$2
    SCRIPT=$3
    ssh "$REMOTE_HOST" "cd $WORK_DIR && sbatch $SCRIPT"
    if [ $? -eq 0 ]; then
        echo "Job submetido com sucesso."
    else
        echo "Erro ao submeter job."
        exit 1
    fi
else
    echo "Ação não reconhecida. Use 'transfer' ou 'execute'."
    exit 1
fi
