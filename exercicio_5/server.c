#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
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

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

int main (int argc, char **argv) {
    int    listenfd, connfd;
    struct sockaddr_in servaddr, client_addr;
    char   buf[MAXDATASIZE];
    char buffer[MAXDATASIZE];
    time_t ticks;


    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Colocando o valor 0 para a escolha aleatória da porta.
    servaddr.sin_port        = htons(8081);
    // final da modificação

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        exit(1);
    }


    // Configuração para o funcionamento do getsockname()
    socklen_t len = sizeof(servaddr);

    if (getsockname(listenfd, (struct sockaddr *)&servaddr, &len) == -1) {
        perror("getsockname");
        exit(1);
    }

    printf("%d\n", ntohs(servaddr.sin_port));
    // final da modificação




    for ( ; ; ) {
      if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
        perror("accept");
        exit(1);
        } 



    //     // Obtendo informações do socket remoto (cliente)
    //     socklen_t client_len = sizeof(client_addr);
    //    if (getpeername(connfd, (struct sockaddr *)&client_addr, &client_len) == -1) {
    //         perror("Falha ao obter informações do socket remoto");
    //         close(connfd);
    //         return -1;
    //     }

    //     printf("Cliente conectado.\n");
    //     printf("Endereço IP do cliente: %s\n", inet_ntoa(client_addr.sin_addr));
    //     printf("Porta do cliente: %d\n", ntohs(client_addr.sin_port));
    //     // final da modificação


    //     // Receber a mensagem enviada pelo cliente
    //     int valread = read(connfd, buffer, MAXDATASIZE);
    //     buffer[valread] = 0;
    //     if (valread > 0) {
    //         printf("Mensagem recebida: %s\n", buffer);
    //     } else {
    //         printf("Falha ao receber a mensagem ou conexão fechada\n");
    //     }
    //     // final da modificação




    //     ticks = time(NULL);
    //     snprintf(buf, sizeof(buf), "Hello from server!\nTime: %.24s\r\n", ctime(&ticks));
    //     write(connfd, buf, strlen(buf));


    }
    sleep(20);
    close(connfd);
    return(0);
}
