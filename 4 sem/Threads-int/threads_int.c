#include "threads_int.h"

/* integral of what function will be calculated */
double function(double x)
{
    return cos( pow(x, 5) * sin(atan(x)) );  
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        ERR_HANDLER("use: ./xxx <NO threads>");

    DEBPRINT("pid = %ld\n", (long)getpid());

    double intVal = calcInt(argv[1], function);

    fprintf(stdout,"\tIntegral value - %lg", intVal);

    return EXIT_SUCCESS;
}

