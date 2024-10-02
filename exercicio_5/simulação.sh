#!/bin/bash

# Endereço IP do servidor e porta
SERVER_IP="127.0.0.1"
SERVER_PORT=8081

# Número mínimo e máximo de clientes
MIN_CLIENTES=5
MAX_CLIENTES=15

# Backlog varia de 0 a 12
BACKLOG_MIN=0
BACKLOG_MAX=12

# Função para executar clientes simultaneamente
function simular_clientes() {
    num_clientes=$1
    backlog=$2

    echo "========================================="
    echo "Iniciando simulação com $num_clientes clientes e backlog de $backlog..."
    echo "========================================="

    # Iniciar o servidor em background com o backlog específico
    ./server $backlog &

    # Capturar o PID do servidor para matar depois
    SERVER_PID=$!

    # Aguardar um pouco para o servidor iniciar
    sleep 1

    # Loop para criar clientes simultaneamente
    for ((i = 0; i < num_clientes; i++)); do
        # Iniciar cliente em background
        ./client $SERVER_IP &
    done

    # Aguardar todos os clientes terminarem
    wait

    # Matar o servidor
    kill $SERVER_PID
    echo "Servidor com backlog de $backlog finalizado."
    echo ""
}

# Loop para variar o backlog e número de clientes
# for backlog in $(seq $BACKLOG_MIN $BACKLOG_MAX); do
#     # Gerar um número aleatório de clientes entre MIN_CLIENTES e MAX_CLIENTES
#     num_clientes=$((RANDOM % (MAX_CLIENTES - MIN_CLIENTES + 1) + MIN_CLIENTES))

#     # Simular clientes
#     simular_clientes $num_clientes $backlog
# done


num_clientes=$1
backlog=$2
simular_clientes $num_clientes $backlog



echo "Simulação finalizada!"
