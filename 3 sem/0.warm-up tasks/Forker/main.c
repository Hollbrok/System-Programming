 #include "libs.h"

enum ERRORS_HANDLER
{
    NOT_ENOUGH_ARGUMENTS = -5,
    TO_MUCH_ARGUMENTS       ,
    ZEROSTRING_ARGV         ,
    INVALID_NO_REQUEST      ,
    FORK_ERROR
};

long getNumber(const char *numString);

int checkNOChilds(long number);

int main(int argc, const char *argv[])
{

    if (argc < 2)
    {
        fprintf(stderr, "Program needs 2 arguments\n");
        exit(NOT_ENOUGH_ARGUMENTS);
    }
    
    if (argc > 2)
    {
        fprintf(stderr, "Too many arguments (need 1 = number of wanted childs)\n");
        exit(TO_MUCH_ARGUMENTS);
    }

    if (*argv[1] == '\0')
    {
        fprintf(stderr, "zero string argv2\n");
        exit(ZEROSTRING_ARGV);
    }

/// 
    long NOChilds = getNumber(argv[1]);

    if (checkNOChilds(NOChilds) == -1)
    {
        fprintf(stderr, "INVALID number of processes requested\n");
        exit(INVALID_NO_REQUEST);
    }

    int anamalyChecker = 1;

    for(size_t i = 0; i < NOChilds; i++)
    {
        pid_t isParent = fork();

        if (isParent == -1)
        {
            perror("fork error\n");
            exit(FORK_ERROR);
        }
        if (!isParent)
        {
            fprintf(stdout, "%d. CHILD: PID  = %d, PPID = %d\n", anamalyChecker, getpid(), getppid());
            
            // if CHILD we need to break because we dont need to create child from the childs
            break;
        }

        ++anamalyChecker;
    }

////
    //fflush(stdout);
    exit(EXIT_SUCCESS);
}

long getNumber(const char *numString)
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

int checkNOChilds(long number)
{
    if (number < 0)
        return -1;



    long maxNOChilds = sysconf(_SC_CHILD_MAX);

    if(number > maxNOChilds)
        return -1;

    return 0;
}
