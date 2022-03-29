#include "../Common/libs.h"
#include "../Common/debug.h"


/* integral of what function will be calculated */
double function(double x)
{
    return cos( pow(x, 5) * sin(atan(x)) );  
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "ERROR. use: %s <NO pc> <NO threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    DEBPRINT("pid = %ld\n", (long)getpid());

    double intVal;//= calcInt(argv[1], function);

    fprintf(stdout,"\tIntegral value - %lg", intVal);

    return EXIT_SUCCESS;
}

