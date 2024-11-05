#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT_TCP 12341
#define PORT_UDP 12346

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


typedef struct {
    int socket;
    char nickname[50];
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_tcp(const char *message, int sender_socket);
void broadcast_udp(const char *message);
void *handle_client(void *arg);
void send_connected_users(int client_socket);

int main() {
    clear_log_file();

    int server_tcp_socket, new_socket, udp_socket;
    struct sockaddr_in server_addr, client_addr, udp_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    fd_set read_fds;

    // Criar socket TCP
    server_tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_tcp_socket < 0) {
        perror("Erro ao criar socket TCP");
        exit(EXIT_FAILURE);
    }

    // Configurar o endereço do servidor TCP
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT_TCP);

    if (bind(server_tcp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind do socket TCP");
        close(server_tcp_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_tcp_socket, MAX_CLIENTS) < 0) {
        perror("Erro no listen");
        close(server_tcp_socket);
        exit(EXIT_FAILURE);
    }

    // Criar socket UDP
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        perror("Erro ao criar socket UDP");
        exit(EXIT_FAILURE);
    }

    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(PORT_UDP);

    if (bind(udp_socket, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
        perror("Erro no bind do socket UDP");
        close(udp_socket);
        exit(EXIT_FAILURE);
    }

    printf("Servidor de bate-papo iniciado (TCP: %d, UDP: %d)\n", PORT_TCP, PORT_UDP);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_tcp_socket, &read_fds);
        FD_SET(udp_socket, &read_fds);

        int max_fd = (server_tcp_socket > udp_socket) ? server_tcp_socket : udp_socket;
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Erro no select");
            continue;
        }

        if (FD_ISSET(server_tcp_socket, &read_fds)) {
            // Aceitar novas conexões TCP
            new_socket = accept(server_tcp_socket, (struct sockaddr *)&client_addr, &addr_len);
            if (new_socket < 0) {
                perror("Erro ao aceitar conexão");
                continue;
            }

            pthread_t tid;
            if (pthread_create(&tid, NULL, &handle_client, (void *)&new_socket) != 0) {
                perror("Erro ao criar thread");
            }
        }
    }

    close(server_tcp_socket);
    close(udp_socket);
    return 0;
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    char nickname[50];


    // Receber o nickname do cliente
    if (recv(client_socket, nickname, sizeof(nickname), 0) <= 0) {
        close(client_socket);
        pthread_exit(NULL);
    }

    // Colocar no Log a mensagem de entrada do cliente
    char log_entry[BUFFER_SIZE];
    snprintf(log_entry, BUFFER_SIZE, "Conexão de %s", nickname);
    log_message(log_entry);

    // Adicionar o cliente à lista
    pthread_mutex_lock(&clients_mutex);
    clients[client_count].socket = client_socket;
    strcpy(clients[client_count].nickname, nickname);
    client_count++;
    pthread_mutex_unlock(&clients_mutex);

    // Enviar notificação de entrada via UDP
    char join_message[BUFFER_SIZE];
    snprintf(join_message, sizeof(join_message), "Usuário %s entrou no chat", nickname);
    broadcast_udp(join_message);
    for (int i = 0; i < client_count; i++) {
        send_connected_users(clients[i].socket);
    }


    // Manter a comunicação com o cliente
    while (1) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }

        buffer[bytes_received] = '\0';

        // Colocar a mensagem enviada no chat no Log
        snprintf(log_entry, BUFFER_SIZE, buffer);
        log_message(log_entry);

        broadcast_tcp(buffer, client_socket);
    }

    // Notificar sobre a saída do cliente
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == client_socket) {
            snprintf(join_message, sizeof(join_message), "Usuário %s saiu do chat", clients[i].nickname);
            broadcast_udp(join_message);

            // Remover cliente da lista
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);


    for (int i = 0; i < client_count; i++) {
        send_connected_users(clients[i].socket);
    }

    close(client_socket);
    pthread_exit(NULL);
}

void broadcast_tcp(const char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket != sender_socket) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void broadcast_udp(const char *message) {
    struct sockaddr_in broadcast_addr;
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(PORT_UDP);
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    int broadcast_enable = 1;
    setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

    sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
    close(udp_socket);
}

void send_connected_users(int client_socket) {
    char user_list[BUFFER_SIZE] = "Usuários conectados: ";
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        strcat(user_list, clients[i].nickname);
        if (i < client_count - 1) strcat(user_list, ", ");
    }
    pthread_mutex_unlock(&clients_mutex);

    send(client_socket, user_list, strlen(user_list), 0);
}
