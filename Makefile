CC = gcc
PROGS = server client
CFLAGS = -lpthread

server : server.c

client : client.c

clean : 
	rm -rf $(PROGS)
