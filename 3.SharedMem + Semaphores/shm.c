#include "libs.h"

#define DEBUG_REGIME 1

#define DEBPRINT(args...)   \
    if(DEBUG_REGIME)        \
        fprintf(stderr, args);

#define ERRCHECK_CLOSE(FD)          \
    do                              \
    {                               \
        if (close(FD) != 0)         \
        {                           \
            fprintf(stderr, #FD);   \
            perror("");             \
        }                           \
    } while(0);

#define PRINT_INT(number)           \
    do                              \
    {                               \
    fprintf(stderr, #number);       \
    fprintf(stderr, " = %ld",       \
            (long) number);         \
    } while (0);

long getNumber(char *numString);

int main(int argc, char* argv[])
{
    


    exit(EXIT_SUCCESS);
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
        // exit(EXIT_FAILURE);
    }
    if (errno != 0)
    {
        fprintf(stderr, "strtol error\n");
        // exit(EXIT_FAILURE);
    }
    
    return gNumber;

}
