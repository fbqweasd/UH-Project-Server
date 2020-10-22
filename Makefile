CC = gcc
PROGS = server client
CFLAGS = -lpthread

all: $(PROGS)

server : server.c Data.h

#client : client.c Data.h

clean : 
	rm -rf $(PROGS)
