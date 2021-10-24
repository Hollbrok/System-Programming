FLAGS  = #-Wimplicit-function-declaration 

.PHONY: all

all: WRITER READER CLEAN

reader.o: reader.c
			gcc -c $(FLAGS) -o reader.o reader.c

writer.o: writer.c
			gcc -c $(FLAGS) -o writer.o writer.c

sem_cntl.o: sem_cntl.c
			gcc -c $(FLAGS) -o sem_cntl.o sem_cntl.c

READER: sem_cntl.o reader.o
			gcc -o reader sem_cntl.o reader.o
			rm -rf reader.o

WRITER: sem_cntl.o writer.o
			gcc -o writer sem_cntl.o writer.o
			rm -rf writer.o

CLEAN:
			rm -rf sem_cntl.o