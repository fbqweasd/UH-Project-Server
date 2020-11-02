CC = gcc
#PROGS = server client
PROGS = server
CFLAGS = -Wall -Wextra -Werror -lpthread -std=c99 -g

all: $(PROGS)

server : main.c Data.h log.c
	$(CC) -o $@ $^ $(CFLAGS)

clean : 
	rm -rf $(PROGS)
