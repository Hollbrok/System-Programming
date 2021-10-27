FLAGS  = #-Wimplicit-function-declaration 

.PHONY: all

all: WRITER READER CLEAN


sem_cntl.o: sem_cntl.c
			gcc -c $(FLAGS) -o sem_cntl.o sem_cntl.c

get_stuff.o: get_stuff.c
			gcc -c $(FLAGS) -o get_stuff.o get_stuff.c

reader.o: reader.c
			gcc -c $(FLAGS) -o reader.o reader.c

writer.o: writer.c
			gcc -c $(FLAGS) -o writer.o writer.c

READER: sem_cntl.o get_stuff.o reader.o
			gcc -o reader sem_cntl.o get_stuff.o reader.o
			rm -rf reader.o

WRITER: sem_cntl.o get_stuff.o writer.o
			gcc -o writer sem_cntl.o get_stuff.o writer.o
			rm -rf writer.o

CLEAN:
			rm -rf sem_cntl.o
			rm -rf get_stuff.o