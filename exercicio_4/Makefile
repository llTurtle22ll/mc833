CFLAGS = -std=c99 -Wall -g
LDFLAGS = -lm

.PHONY: all clean

all: client

client: client.c server.c
	gcc $(CFLAGS) client.c -o client.exe $(LDFLAGS)
	gcc $(CFLAGS) server.c -o server.exe $(LDFLAGS)

clean:
	rm -f client.exe
	rm -f server.exe