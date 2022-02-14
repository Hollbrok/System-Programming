gcc --coverage test.c ../bintree_elem_func.c ../bintree_func.c -o test
./test
lcov -t "test" -o test.info -c -d .
genhtml -o report test.info
