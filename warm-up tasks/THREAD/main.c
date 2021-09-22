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

void *threadStuff(void *);


int TESTVAL = 0;

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
    long NOThreads = getNumber(argv[1]);

    int funcArg = 0;

    if (NOThreads < 0)
    {
        fprintf(stderr, "Less then 0 NPThreads\n");
        exit(EXIT_FAILURE);
    }
    if(NOThreads > 1000000)
    {
        fprintf(stderr, "Too big NOThreads\n");
        exit(EXIT_FAILURE);
    }


    for(size_t i = 0; i < NOThreads; i++)
    {
        pthread_t thread;

        int isError= pthread_create(&thread, NULL, threadStuff, &funcArg);

        if (isError)
        {
            fprintf(stderr, "ERROR pthread_create\n");
            exit(EXIT_FAILURE);
        }

        if (pthread_join(thread, NULL) != 0)
        {
            fprintf(stderr, "ERROR pthread_join\n");
            exit(EXIT_FAILURE);
        }   
    }

    fprintf(stdout, "VAL = %d\n", TESTVAL);

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

void *threadStuff(void * arg)
{
    TESTVAL += 10000;
    return NULL;
}


