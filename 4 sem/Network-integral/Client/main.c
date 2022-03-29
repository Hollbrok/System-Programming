#include "../Common/libs.h"
#include "../Common/debug.h"

int main(int argc, char *argv[])
{
    if (argc != 1)
    {
        fprintf(stderr, "ERROR. use: %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    DEBPRINT("pid = %ld\n", (long)getpid());

    double intVal;//= calcInt(argv[1], function);

    fprintf(stdout,"\tIntegral value - %lg", intVal);

    return EXIT_SUCCESS;
}