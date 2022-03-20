#include "threads_int.h"

/* integral of what function will be calculated */
double function(double x)
{
    return cos( pow(x, 5) * sin(atan(x)) );  
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        ERR_HANDLER("./xxx <NO threads>Z");

    calcInt(argv[1], function);

    return EXIT_SUCCESS;
}

