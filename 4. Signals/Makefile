FLAGS  = #-Wimplicit-function-declaration 

.PHONY: all

all: MAIN


MAIN: sem_cntl.o get_stuff.o reader.o
			gcc -o reader sem_cntl.o get_stuff.o reader.o
			rm -rf reader.o


CLEAN:
			rm -rf sem_cntl.o
			rm -rf get_stuff.o