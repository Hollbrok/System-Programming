#include "libs.h" 

// enum

enum ERRORS_FORK 
{
    NOT_ENOUGH_ARGUMENTS = -5,
    TO_MUCH_ARGUMENTS    = -6,
    FORK_ERROR           = -7,
    INVALID_NOPROCESS    = -8,
    ERROR_SYSCONF        = -9,
    ZEROSTRING_ARGV      = -10
};



long getNumber(char *numString);

int checkNOProcesses(long NOProcesses);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Program needs 2 arguments\n");
        exit(NOT_ENOUGH_ARGUMENTS);
    }
    
    if (argc > 2)
    {
        fprintf(stderr, "Too much arguments (need 2)\n");
        exit(TO_MUCH_ARGUMENTS);
    }

    if (*argv[1] == '\0')
    {
        fprintf(stderr, "zero string argv2\n");
        exit(ZEROSTRING_ARGV);
    }

    long NOProcesses = getNumber(argv[1]);

    if(checkNOProcesses(NOProcesses) != 0) // checking valid or invalid NOProcess is
    {
        fprintf(stderr, "Invalid number of processes\n");
        exit(INVALID_NOPROCESS);
    }

    pid_t isParent;

    for (long i = 0; i < NOProcesses; i++)
    {    
        isParent = fork();
    
        if (isParent == -1)
        {
            fprintf(stderr, "Error in fork()\n");
            exit(FORK_ERROR);
        }
        if (!isParent)
        {
            fprintf(stdout, "%ld. CHILD, PID: %d\n", i + 1, getpid());
            //fflush(stdout);
            break;
        }
        else // if(isParent == getpid())  --> Parent
        {
            fprintf(stdout, "PARENT, PID: %d\n", getpid());
            //fflush(stdout);
        }

    }

    //printf("\n");
    return 1;
}

long getNumber(char *numString)
{
    if (*numString == '\0')
    {
        fprintf(stderr, "empty number argument\n");
        exit(EXIT_FAILURE);
    }

    errno = 0;

    long gNumber;
    char* endOfEnter;

    const int baseOfNumber = 10;
    gNumber = strtol(numString, &endOfEnter, baseOfNumber);

    if(*endOfEnter != '\0')
    {
        fprintf(stderr, "strtol error\n");
        exit(EXIT_FAILURE);
    }
    if (errno != 0)
    {
        fprintf(stderr, "strtol error\n");
        exit(EXIT_FAILURE);
    }
    
    return gNumber;

}

int checkNOProcesses(long NOProcesses)
{
    errno = 0;

    pid_t curPID = getpid();

    int maxNOProcesses = sysconf(_SC_CHILD_MAX);

    if (maxNOProcesses == -1)
    {
        if (errno != 0)
            fprintf(stderr, "INVALID NAME\n");
        else
            fprintf(stderr, "limit is indeterminate\n");
    
        return -1;//exit(ERROR_SYSCONF);
    }

    if (NOProcesses + curPID > maxNOProcesses) // check if total number of processes at the end will not be more than maxNOProcesses
    {
        fprintf(stderr, "Too big number of simultaneous processes (current max = max - curPID = %d - %d = %d)\n",
                maxNOProcesses, curPID, maxNOProcesses - curPID);
        return -1;
    }

    if (NOProcesses < 0)
    {
        fprintf(stderr, "Invalid value of 2nd argumnet (must be >= 0)\n");
        return -1;    
    }

    return 0;

}