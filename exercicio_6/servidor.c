#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#define LISTENQ 10
#define MAXDATASIZE 1024

// Início da modificação
// Função para gerar as informações de monitoramento
void generate_monitoring_info(char *buffer, struct sockaddr_in *servaddr, char* addr) {
    int cpu_load = rand() % 100;
    int memory_usage = rand() % 100;
    const char *status = (rand() % 2) ? "Ativo" : "Inativo";

    time_t ticks = time(NULL);
    snprintf(buffer, MAXDATASIZE,
             "Monitoramento do servidor:\nIP: %s\nPorta: %d\nHorário: %.24s\nCPU: %d%%\nMemória: %d%%\nStatus: %s\n",
             addr, ntohs(servaddr->sin_port), ctime(&ticks),
             cpu_load, memory_usage, status);
}
// Final da modificação

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr, client_addr;
    char buf[MAXDATASIZE], buffer[MAXDATASIZE];

    srand(time(NULL));

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(0);

    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);

    socklen_t len = sizeof(servaddr);
    getsockname(listenfd, (struct sockaddr *)&servaddr, &len);
    printf("Porta do servidor: %d\n", ntohs(servaddr.sin_port));

    // Início da modificação
    // Loop para enviar os dados do sistema 
    for (;;) {
        socklen_t client_len = sizeof(client_addr);

        // Obtendo informações do socket local
        if (getsockname(listenfd, (struct sockaddr *)&servaddr, &client_len) == -1) {
            perror("Falha ao obter informações do socket local");
            close(listenfd);
            exit(1);
        }

        connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len);

        // Gera e envia dados de monitoramento para o cliente
        generate_monitoring_info(buf, &servaddr, inet_ntoa(client_addr.sin_addr));
        write(connfd, buf, strlen(buf));

        // Recebe mensagem do cliente e ecoa de volta
        int valread = read(connfd, buffer, MAXDATASIZE);
        while (valread > 0) {
            buffer[valread] = '\0';
            write(connfd, buffer, valread);  // Echo
            valread = read(connfd, buffer, MAXDATASIZE);
        }
        close(connfd);
    }
    // Final da modificação
    return 0;
}
