all: test

test : main.o bt.o bt_elem.o
	gcc -o out *.o

main.o : main.c	
	gcc -c -o main.o main.c

bt.o : bintree_func.c	
	gcc -c -o bt.o bintree_func.c

bt_elem.o : bintree_elem_func.c	
	gcc -c -o bt_elem.o bintree_elem_func.c

#CFLAGS = -g -MD -c

#%.o: %.c 
#		$(CC) $(CFLAGS) -o $@ $<


#-include *.d

#.PHONY: clean
#clean: rm *.o *.d *.gcda *.gcno *.gcov *.out *.dot *.jpg

