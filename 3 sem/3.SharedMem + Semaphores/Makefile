FLAGS  = #-Wimplicit-function-declaration 

.PHONY: all

all: WRITER READER CLEAN


get_stuff.o: get_stuff.c
			gcc -c $(FLAGS) -o get_stuff.o get_stuff.c

reader.o: reader.c
			gcc -c $(FLAGS) -o reader.o reader.c

writer.o: writer.c
			gcc -c $(FLAGS) -o writer.o writer.c

READER: get_stuff.o reader.o
			gcc -o reader  get_stuff.o reader.o
			rm -rf reader.o

WRITER: get_stuff.o writer.o
			gcc -o writer get_stuff.o writer.o
			rm -rf writer.o

CLEAN:
			rm -rf get_stuff.o