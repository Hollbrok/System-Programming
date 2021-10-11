TARGET = $(f)
FLAGS  = #-Wimplicit-function-declaration 

.PHONY: all

all: $(TARGET)

$(f).o: $(f).c
			gcc -c $(FLAGS) -o $(f).o $(f).c

sem_cntl.o: sem_cntl.c
			gcc -c $(FLAGS) -o sem_cntl.o sem_cntl.c

$(TARGET): sem_cntl.o $(f).o
			gcc -o $(TARGET) sem_cntl.o $(f).o
			rm -rf hello *.o