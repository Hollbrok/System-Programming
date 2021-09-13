#include "libs.h" 

const int MY_NOT_ENOUGH_ARGUMENTS = -5;
const int MY_TO_MUCH_ARGUMENTS    = -6;
const int MY_FORK_ERROR           = -7;
const int MY_INVALID_NOPROCESS    = -8;
const int MY_ERROR_SYSCONF        = -9;

long getNumber(char *numString);

void checkNOProcesses(long NOProcesses);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Program needs 2 arguments\n");
        exit(MY_NOT_ENOUGH_ARGUMENTS);
    }
    
    if (argc > 2)
    {
        fprintf(stderr, "Too much arguments (need 2)\n");
        exit(MY_TO_MUCH_ARGUMENTS);
    }

    long NOProcesses = getNumber(argv[1]);

    checkNOProcesses(NOProcesses); // checking valid or invalid NOProcess is

    pid_t isParent;

    for (long i = 0; i < NOProcesses; i++)
    {    
        isParent = fork();
    
        if (isParent == -1)
        {
            fprintf(stderr, "Error in fork()\n");
            exit(MY_FORK_ERROR);
        }
        if (!isParent)
        {
            printf("%ld. NOT Parent, PID: %d\n ", i + 1, getpid());
            break;
        }
        else // Parent
        {
            printf("PARENT, PID: %d\n", getpid());
        }

    }

    printf("\n");
    return 1;
}

long getNumber(char *numString)
{
    /*if (numString == NULL)
    {
        fprintf(stderr, "null string argument\n");
    }*/
    if (*numString == '\0')
    {
        fprintf(stderr, "empty number argument\n");
        // exit(EXIT_FAILURE);
    }

    errno = 0;

    long gNumber;
    char* endOfEnter;

    const int baseOfNumber = 10;
    gNumber = strtol(numString, &endOfEnter, baseOfNumber);

    if(*endOfEnter != '\0')
    {
        fprintf(stderr, "strtol error\n");
        // exit(EXIT_FAILURE);
    }
    if (errno != 0)
    {
        fprintf(stderr, "strtol error\n");
        // exit(EXIT_FAILURE);
    }
    
    return gNumber;

}

void checkNOProcesses(long NOProcesses)
{
    errno = 0;

    int maxNOProcesse = sysconf(_SC_CHILD_MAX);

    if (maxNOProcesse == -1)
    {
        if (errno != 0)
            fprintf(stderr, "INVALID NAME\n");
        else
            fprintf(stderr, "limit is indeterminate\n");
    
        exit(MY_ERROR_SYSCONF);
    }

    if (NOProcesses > maxNOProcesse)
    {
        fprintf(stderr, "Too big number of simultaneous processes (max = %d)\n", maxNOProcesse);
        exit(MY_INVALID_NOPROCESS);
    }

    if (NOProcesses < 0)
    {
        fprintf(stderr, "Invalid value of 2nd argumnet (must be >= 0)\n");
        exit(MY_INVALID_NOPROCESS);
    }

}