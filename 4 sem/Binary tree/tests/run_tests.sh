make
cd ./tests
gcc --coverage ../src/bintree_elem_func.c ../src/bintree_func.c ./test.c -o ./test
./test
lcov -t "test" -o test.info -c -d . --rc lcov_branch_coverage=1 --rc lcov_function_coverage=1
genhtml -o report test.info
opera report/index.html
make clean