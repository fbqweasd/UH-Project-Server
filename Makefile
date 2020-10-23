CC = gcc
#PROGS = server client
PROGS = server
CFLAGS = -lpthread

all: $(PROGS)

server : server.c Data.h log.c

#client : client.c Data.h

clean : 
	rm -rf $(PROGS)
