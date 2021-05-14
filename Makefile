CC = gcc
#PROGS = server client
PROGS = server
CFLAGS = -Wall -Wextra -Werror -lpthread -std=c99 -g

IncludePath = -I /opt/ext/json-c/include
Lib = -l json-c -L /opt/ext/json-c/lib -Wl,-rpath,/opt/ext/json-c/lib/

all: $(PROGS)

server : main.c packet.h log.c
	$(CC) -o $@ $^ $(CFLAGS) $(IncludePath) $(Lib)

clean : 
	rm -rf $(PROGS)
