#include "libs.h"
#include "debug.h"
 
long getNumber(char *numString);

int main(int argc, char *argv[])
{
   

    exit(EXIT_SUCCESS);
}

long getNumber(char *numString)
{
    if (numString == NULL)
    {
        fprintf(stderr, "null string argument\n");
    }
    if (*numString == '\0')
    {
        fprintf(stderr, "empty number argument\n");
        exit(ERR_ARGS);
    }

    errno = 0;

    long gNumber;
    char* endOfEnter;

    const int baseOfNumber = 10;
    gNumber = strtol(numString, &endOfEnter, baseOfNumber);

    if(*endOfEnter != '\0')
    {
        fprintf(stderr, "strtol error\n");
        exit(ERR_ARGS);
    }
    if (errno != 0)
    {
        fprintf(stderr, "strtol error\n");
        exit(ERR_ARGS);
    }
    
    return gNumber;

}