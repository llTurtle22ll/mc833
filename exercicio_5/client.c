#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 4096

#define BUFFER_SIZE 1024

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

int main(int argc, char **argv) {
    int    sockfd;
    char   error[MAXLINE + 1];
    struct sockaddr_in servaddr, local_addr;
    char buffer[BUFFER_SIZE];

    if (argc != 2) {
        strcpy(error,"uso: ");
        strcat(error,argv[0]);
        strcat(error," <IPaddress>\n");
        perror(error);
        exit(1);
    }

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(8081);

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }

    sleep(5);



    // // Obtendo informações do socket local
    // socklen_t addr_len = sizeof(local_addr);

    // if (getsockname(sockfd, (struct sockaddr *)&local_addr, &addr_len) == -1) {
    //     perror("Falha ao obter informações do socket local");
    //     close(sockfd);
    //     return -1;
    // }

    // // Imprimindo as informações do socket local
    // printf("Endereço IP local: %s\n", inet_ntoa(local_addr.sin_addr));
    // printf("Porta local: %d\n", ntohs(local_addr.sin_port));
    // // final da modificação





    // // Receber do stdin a mensagem a ser enviada para o servidor
    // printf("Digite a mensagem para enviar ao servidor: ");
    // fgets(buffer, BUFFER_SIZE-1, stdin);


    // // Enviando a mensagem para o servidor
    // send(sockfd, buffer, strlen(buffer), 0);
    // printf("Mensagem enviada.\n");
    // // final da modificação


    close(sockfd);
}
