VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all

CODE_PATH = ./src
TEST_PATH = ./tests

all : main.o bt.o bt_elem.o
	gcc -o out *.o && $(MAKE) clean

main.o : $(CODE_PATH)/main.c	
	gcc -c -o main.o $(CODE_PATH)/main.c

bt.o : $(CODE_PATH)/bintree_func.c	
	gcc -c -o bt.o $(CODE_PATH)/bintree_func.c

bt_elem.o : $(CODE_PATH)/bintree_elem_func.c	
	gcc -c -o bt_elem.o $(CODE_PATH)/bintree_elem_func.c

run_test :
	sh tests/run_tests.sh 

valgrind : all
	valgrind $(VALGRIND_FLAGS) ./out

#CFLAGS = -g -MD -c

#%.o: %.c 
#		$(CC) $(CFLAGS) -o $@ $<


#-include *.d

.PHONY: clean
clean: 
		rm *.o *.d *.gcda *.gcno *.gcov *.out *.dot *.jpg

