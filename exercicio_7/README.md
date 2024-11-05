## Instruções para compilação

Dentro do arquivo ZIP enviado, existe um Makefile, portanto só é necessário rodar o comando "make" no terminal.
Caso não tenha "make" instalado na sua máquina, utilize os seguintes comandos:

Cliente: 
	gcc  -std=c99 -Wall -g cliente.c -o cliente -lm

Servidor:
	gcc  -std=c99 -Wall -g servidor.c -o servidor -lm

## Instruções para a execução
Para iniciar o servidor, após a compilação, rode o executável do servidor da seguinte maneira:

"./servidor"

Sempre que o servidor é reiniciado, o arquivo de log é limpo, iniciando um novo arquivo de log.

Para iniciar um cliente, utilize o comando:
"./cliente 127.0.0.1 <NICKNAME>"

onde NICKNAME é o nome escolhido pelo usuário que será exibido para os demais clientes conectados no servidor de chat.
