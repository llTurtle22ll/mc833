CFLAGS = -std=c99 -Wall -g
LDFLAGS = -lm

.PHONY: all clean

all: cliente

cliente: cliente.c servidor.c
	gcc $(CFLAGS) cliente.c -o cliente.exe $(LDFLAGS)
	gcc $(CFLAGS) servidor.c -o servidor.exe $(LDFLAGS)

clean:
	rm -f cliente.exe
	rm -f servidor.exe