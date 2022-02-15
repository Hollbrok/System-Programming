VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all

CFLAGS = -g -MD -c
GCOV_FLAGS = -fprofile-arcs -ftest-coverage

CODE_PATH = src
TEST_PATH = tests

SOURCES = $(CODE_PATH)/main.c $(CODE_PATH)/bintree_func.c $(CODE_PATH)/bintree_elem_func.c

OBJECTS = $(SOURCES:.c=.o)

EXECUTABLE = xxx

# -------------------------------

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<


run_test :
	sh tests/run_tests.sh 

valgrind : $(SOURCES)
	valgrind $(VALGRIND_FLAGS) ./xxx


-include *.d

.PHONY: clean
clean: 
		cd src/ && rm -rf *.o *.d *.gcda *.gcno *.gcov *.out *.dot *.jpg




