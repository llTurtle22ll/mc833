#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <IP_do_Servidor> <Porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int client_sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Criar o socket do cliente
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Erro ao criar o socket");
        exit(EXIT_FAILURE);
    }

    // Configurar o endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // Conectar ao servidor
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Erro ao conectar ao servidor");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    // Exibir informações sobre a conexão
    printf("Conectado ao servidor %s:%d\n", server_ip, server_port);

    // Receber tarefa do servidor
    int bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
    while (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Tarefa recebida: %s\n", buffer);

        // Simular processamento da tarefa
        sleep(5);

        // Enviar resposta para o servidor
        char *response = "TAREFA_LIMPEZA CONCLUÍDA";
        send(client_sock, response, strlen(response), 0);
        
    }


    // Fechar a conexão
    close(client_sock);
    return 0;
}
