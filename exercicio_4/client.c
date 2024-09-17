#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

// Criar socket
int Socket(int family, int type, int flag) {
    int server_sock;
    if ((server_sock = socket(family, type, flag)) == -1) {
        perror("Erro ao criar o socket");
        exit(EXIT_FAILURE);
    } else {
        return server_sock;
    }
}

int Connect(int client_sock, struct sockaddr_in server_addr) {
    // Conectar ao servidor
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Erro ao conectar ao servidor");
        close(client_sock);
        exit(EXIT_FAILURE);
    }
    
    return 0;
}

int Getsockname(int client_sock, struct sockaddr_in *client_addr, socklen_t *client_addr_len) {
    //Adquirir informações sobre a conexão
    if (getsockname(client_sock, (struct sockaddr *)client_addr, client_addr_len) == -1) {
        perror("Erro ao obter o endereço do cliente");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <IP_do_Servidor> <Porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Criar o socket do cliente
    client_sock = Socket(AF_INET, SOCK_STREAM, 0);

    // Configurar o endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    Connect(client_sock, server_addr);

    Getsockname(client_sock, &client_addr, &client_addr_len);

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    // Exibir informações sobre a conexão
    printf("Conectado ao servidor %s:%d\n", server_ip, server_port);
    printf("Informações da conexão %s:%d\n", client_ip, ntohs(client_addr.sin_port));

    while (1)
    {
        // Receber tarefa do servidor
        int bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            
            buffer[bytes_received] = '\0';
            printf("Tarefa recebida: %s\n", buffer);

            // Simular processamento da tarefa
            sleep(5);

            //Enviar a resposta de desconexão para o servidor
            if (strcmp("ENCERRAR", buffer) == 0)
            {
                char *response = "CONEXÃO ENCERRADA";
                send(client_sock, response, strlen(response), 0);
                break;
            }

            // Enviar resposta para o servidor
            char *response = "TAREFA_LIMPEZA CONCLUÍDA";
            send(client_sock, response, strlen(response), 0);
            
        }
    }
    


    // Fechar a conexão
    close(client_sock);
    return 0;
}
