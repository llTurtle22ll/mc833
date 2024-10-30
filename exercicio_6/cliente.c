#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/select.h>

#define MAXLINE 4096
#define BUFFER_SIZE 1024

// Início da modificação
// Refatoração do código para ficar mais legível
void configure_socket(int *sockfd, struct sockaddr_in *servaddr, const char *ip, int port) {
    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }

    servaddr->sin_family = AF_INET;
    servaddr->sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &servaddr->sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

    if (connect(*sockfd, (struct sockaddr *) servaddr, sizeof(*servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }
}
// Final da modificação

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "uso: %s <IP1> <Port1> <Port2> <arquivo de entrada>\n", argv[0]);
        exit(1);
    }

    int sockfd1, sockfd2, maxfd;
    struct sockaddr_in servaddr1, servaddr2;
    FILE *input_file = fopen(argv[4], "r");
    if (!input_file) {
        perror("Erro ao abrir o arquivo de entrada");
        exit(1);
    }

    // Configurar as duas conexões
    configure_socket(&sockfd1, &servaddr1, argv[1], atoi(argv[2]));
    configure_socket(&sockfd2, &servaddr2, argv[1], atoi(argv[3]));

    // Preparar para monitoramento com select()
    fd_set read_fds1, read_fds2;
    char buffer[BUFFER_SIZE], response[BUFFER_SIZE];
    int bytes_read, total_sent = 0, total_received = 0;

    while (fgets(buffer, BUFFER_SIZE, input_file) != NULL) {
        // Início da modificação
        // Enviar a linha para ambos os servidores
        send(sockfd1, buffer, strlen(buffer), 0);
        send(sockfd2, buffer, strlen(buffer), 0);
        total_sent += 2;

        // Monitorar as respostas
        FD_ZERO(&read_fds1);
        FD_ZERO(&read_fds2);
        FD_SET(sockfd1, &read_fds1);
        FD_SET(sockfd2, &read_fds2);
        maxfd = sockfd1 > sockfd2 ? sockfd1 : sockfd2;

        // Monitoramento dos sockets
        if (select(maxfd + 1, &read_fds1, NULL, NULL, NULL) < 0) {
            perror("select error");
            exit(1);
        }
        // Monitoramento dos sockets
        if (select(maxfd + 1, &read_fds2, NULL, NULL, NULL) < 0) {
            perror("select error");
            exit(1);
        }

        // Ler respostas dos servidores
        if (FD_ISSET(sockfd1, &read_fds1)) {
            bytes_read = recv(sockfd1, response, BUFFER_SIZE, 0);
            if (bytes_read > 0) {
                response[bytes_read] = '\0';
                printf("Resposta do Servidor 1: %s\n", response);
                total_received++;
            }
        }
        if (FD_ISSET(sockfd2, &read_fds2)) {
            bytes_read = recv(sockfd2, response, BUFFER_SIZE, 0);
            if (bytes_read > 0) {
                response[bytes_read] = '\0';
                printf("Resposta do Servidor 2: %s\n", response);
                total_received++;
            }
        }
        // Final da modificação
    }

    // Fechar arquivo e conexões
    fclose(input_file);
    shutdown(sockfd1, SHUT_RDWR);
    shutdown(sockfd2, SHUT_RDWR);
    close(sockfd1);
    close(sockfd2);
    return 0;
}
