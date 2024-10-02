CFLAGS = -std=c99 -Wall -g
LDFLAGS = -lm

.PHONY: all clean

all: client

client: client.c server.c
	gcc $(CFLAGS) client.c -o client $(LDFLAGS)
	gcc $(CFLAGS) server.c -o server $(LDFLAGS)

clean:
	rm -f client
	rm -f server