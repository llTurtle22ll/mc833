#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define LOG_FILE "log.txt"


// Função para deletar o log
void clear_log_file() {
    FILE *file = fopen(LOG_FILE, "w");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo de log");
        exit(EXIT_FAILURE);
    }
    fclose(file);
}

// Função para logar mensagens
void log_message(const char *message) {
    FILE *file = fopen(LOG_FILE, "a");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo de log");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s\n", message);
    fclose(file);
}

// Função que decide aleatoriamente se envia uma nova tarefa ou encerra a conexão
int should_send_task() {
    srand(time(NULL));
    int decision = rand() % 100;  // Retorna 0 ou 1 aleatoriamente

    return decision < 80;
}

// Função que trata cada conexão do cliente
void handle_client(int client_sock, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);

    // Log da conexão do cliente
    char log_entry[BUFFER_SIZE];
    snprintf(log_entry, BUFFER_SIZE, "Conexão de %s:%d", client_ip, client_port);
    log_message(log_entry);

    while (1) {
        // Decisão aleatória: enviar tarefa ou encerrar a conexão
        if (should_send_task()) {
            // Enviar uma tarefa para o cliente
            char *task = "TAREFA: LIMPEZA";
            send(client_sock, task, strlen(task), 0);
            snprintf(log_entry, BUFFER_SIZE, "Tarefa enviada para %s:%d: TAREFA_LIMPEZA", client_ip, client_port);
            log_message(log_entry);

            // Aguardar resposta do cliente
            int bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
            while  (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    // printf("\nDEBUG LOG\n");
                    snprintf(log_entry, BUFFER_SIZE, "Resposta do cliente %s:%d: %.980s", client_ip, client_port, buffer);
                    log_message(log_entry);
                    break;
                }
            
        } else {
            // Encerrar a conexão com o cliente
            char *task = "ENCERRAR";
            send(client_sock, task, strlen(task), 0);
            snprintf(log_entry, BUFFER_SIZE, "Instrução enviada para %s:%d: ENCERRAR", client_ip, client_port);
            log_message(log_entry);
            int bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                // printf("\nDEBUG LOG ENCERRAR\n");
                snprintf(log_entry, BUFFER_SIZE, "Resposta do cliente %s:%d: %.980s", client_ip, client_port, buffer);
                log_message(log_entry);
            }
            break;
        }
    }

    // Log de desconexão
    snprintf(log_entry, BUFFER_SIZE, "Desconexão de %s:%d", client_ip, client_port);
    log_message(log_entry);

    close(client_sock);
}

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

int Bind(struct sockaddr_in server_addr, int server_sock) {
    // Associar o socket ao endereço e porta
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Erro no bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    } else {
        return 0;
    }
}

int Listen(int server_sock) {
    // Escutar por conexões
    if (listen(server_sock, 5) == -1) {
        perror("Erro no listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    } else {
        return 0;
    }
}

int Accept(int server_sock, struct sockaddr_in *client_addr, socklen_t *addr_len) {
    int client_sock;
    // Aceitar conexão de um cliente
    if ((client_sock = accept(server_sock, (struct sockaddr *)client_addr, addr_len)) == -1) {
        perror("Erro no accept");
    }

    return client_sock;
}

int main(int argc, char *argv[]) {
    clear_log_file();
    
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Inicializa o gerador de números aleatórios
    srand(time(NULL));

    server_sock = Socket(AF_INET, SOCK_STREAM, 0);

    // Configurar o endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    Bind(server_addr, server_sock);

    Listen(server_sock);

    log_message("Servidor iniciado e aguardando conexões...");

    while (1) {
        client_sock = Accept(server_sock, &client_addr, &addr_len);

        // Criar um processo filho para lidar com o cliente
        if (fork() == 0) {
            // Processo filho
            close(server_sock); // O processo filho não precisa do socket do servidor
            handle_client(client_sock, client_addr);
            exit(0); // Encerra o processo filho após lidar com o cliente
        }

        // O processo pai fecha o socket do cliente (pois o filho já está lidando com ele)
        close(client_sock);
    }

    close(server_sock);
    return 0;
}
