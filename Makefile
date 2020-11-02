CC = gcc
#PROGS = server client
PROGS = server
CFLAGS = -lpthread

all: $(PROGS)

server : main.c Data.h log.c
	$(CC) -o #@ #^ $(CFLAGS)

clean : 
	rm -rf $(PROGS)
