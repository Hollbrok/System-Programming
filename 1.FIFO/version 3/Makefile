FLAGS  = #-Wimplicit-function-declaration 

.PHONY: all

all: client.o server.o

client.o: client.c
			g++ $(FLAGS) -o client client.c
server.o: server.c
			g++ $(FLAGS) -o server server.c
