#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define PORT_TCP 12341
#define PORT_UDP_MIN 10000  // Porta mínima para a escolha aleatória
#define PORT_UDP_MAX 11000  // Porta máxima para a escolha aleatória

void *receive_messages(void *socket_desc);
void *receive_udp_notifications(void *udp_socket_desc);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <IP do Servidor> <Nickname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    char *nickname = argv[2];

    int tcp_socket;
    struct sockaddr_in server_addr;

    // Criar socket TCP
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        perror("Erro ao criar socket TCP");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_TCP);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // Conectar ao servidor
    if (connect(tcp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }

    // Enviar nickname para o servidor
    send(tcp_socket, nickname, strlen(nickname), 0);

    printf("Conectado ao servidor de bate-papo como '%s'\n", nickname);


    char nick2[52];
    strcpy(nick2, nickname);
    strcat(nick2, ": ");

    // Criar thread para receber mensagens TCP
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, (void *)&tcp_socket) != 0) {
        perror("Erro ao criar thread de recepção de mensagens");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }

    // Criar socket UDP para receber notificações
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        perror("Erro ao criar socket UDP");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    int udp_port = PORT_UDP_MIN + rand() % (PORT_UDP_MAX - PORT_UDP_MIN + 1);

    struct sockaddr_in udp_addr;
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(udp_port);
    udp_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(udp_socket, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
        perror("Erro no bind do socket UDP");
        close(udp_socket);
        exit(EXIT_FAILURE);
    }

    // Criar thread para receber notificações UDP
    pthread_t udp_thread;
    if (pthread_create(&udp_thread, NULL, receive_udp_notifications, (void *)&udp_socket) != 0) {
        perror("Erro ao criar thread de recepção de notificações UDP");
        close(udp_socket);
        exit(EXIT_FAILURE);
    }

    // Enviar mensagens ao servidor
    char message[BUFFER_SIZE];
    char message_2[BUFFER_SIZE+52];
    char message_disconnect[BUFFER_SIZE+52];
    while (1) {
        strcpy(message_2, "");
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = 0;  // Remover o '\n' do final

        if (strcmp(message, "/sair") == 0) {
            printf("Desconectando...\n");
            strcpy(message_disconnect, "Usuário desconectado: ");
            strcat(message_disconnect, nickname);
            
            send(tcp_socket, message_disconnect, strlen(message_disconnect), 0);

            break;
        }

        strcpy(message_2, nick2);

        strcat(message_2, message);

        send(tcp_socket, message_2, strlen(message_2), 0);
    }

    // Fechar conexões e sair
    close(tcp_socket);
    close(udp_socket);
    pthread_cancel(receive_thread);
    pthread_cancel(udp_thread);
    pthread_join(receive_thread, NULL);
    pthread_join(udp_thread, NULL);

    return 0;
}

void *receive_messages(void *socket_desc) {
    int socket = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
    }

    return NULL;
}

void *receive_udp_notifications(void *udp_socket_desc) {
    int udp_socket = *(int *)udp_socket_desc;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);
    int bytes_received;

    while ((bytes_received = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0,
                                      (struct sockaddr *)&sender_addr, &sender_len)) > 0) {
        buffer[bytes_received] = '\0';
        printf("NOTIFICAÇÃO: %s\n", buffer);
    }

    return NULL;
}
