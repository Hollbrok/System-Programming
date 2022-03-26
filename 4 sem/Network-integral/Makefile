# flags

VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all

CFLAGS = -g -MD -c 
ADDIT_FLAGS = -pthread -lm

GCOV_FLAGS = -fprofile-arcs -ftest-coverage

# pathes

CODE_PATH = .

SOURCES = $(CODE_PATH)/network_int.c $(CODE_PATH)/network_int_func.c

OBJECTS = $(SOURCES:.c=.o)

# names of output files

EXECUTABLE = xxx

all: exec run_tests

# compile stuff

exec: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
		$(CC) $(OBJECTS) $(ADDIT_FLAGS) -o $@

%.o: %.c
		$(CC) $(CFLAGS) -o $@ $<

-include *.d

# for testing speed 

maxThreads = 16

run_tests:
		sh ./test_speed.sh $(maxThreads)

# clean stuff

.PHONY: clean

clean:
	rm -rf *.o

