# flags

VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all

CFLAGS = -g -MD -c 
ADDIT_FLAGS = -pthread -lm

GCOV_FLAGS = -fprofile-arcs -ftest-coverage

# pathes

CODE_PATH = .

SOURCES = $(CODE_PATH)/threads_int.c

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

max_NO_threads = 16

run_tests:
		sh ./test_speed.sh $(max_NO_threads)

# clean stuff

.PHONY: clean

clean:
	rm -rf *.o

