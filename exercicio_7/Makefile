CFLAGS = -std=c99 -Wall -g
LDFLAGS = -lm

.PHONY: all clean

all: cliente

cliente: cliente.c servidor.c
	gcc $(CFLAGS) cliente.c -o cliente $(LDFLAGS)
	gcc $(CFLAGS) servidor.c -o servidor $(LDFLAGS)

clean:
	rm -f cliente
	rm -f servidor