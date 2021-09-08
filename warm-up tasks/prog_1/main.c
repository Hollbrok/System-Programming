#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

long getNumber(char *numString);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Please, enter a number\n");
        // exit(EXIT_FAILURE);
    }
    
    if (argc > 2)
    {
        fprintf(stderr, "To many arguments");
        // exit(EXIT_FAILURE);
    }

    long maxNumber = getNumber(argv[1]);

    if(maxNumber < 1)
    {
        fprintf(stderr, "Incorrect number\n");
        // exit(EXIT_FAILURE);
    }

    for(long i = 1; i <= maxNumber; i++)
        printf("%ld ", i);

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