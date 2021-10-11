//#include "libs.h"

int fileDump = 0;

int argsHandler(int NOargs, int needFlags, int argc, const char *argv[]);

int main(int argc, const char *argv[])
{
     int needFlag = 1;

    if ( argsHandler(1, needFlag, argc, argv) != 0)
    {
        fprintf(stderr, "ERROR in argsHANDLER\n");
        exit(EXIT_FAILURE);
    }

    int outFile = 1;//stdout;

    if (fileDump)
        outFile = open("dump", O_WRONLY | O_CREAT, S_IRWXU);

    close(outFile);

    // ITS just example of how to change OUTPUT file (for debug if u want) by the using flags in console.
    // There is NO all errors handler, etc.
    // NEED to add MORE flexibility of the program 
}

int argsHandler(int NOargs, int needFlags, int argc, const char *argv[])
{
    
    if ((argc != (NOargs + 1) + (needFlags ? 1 : 0 ) ) )
    {
        fprintf(stderr, "inval number of arguments\n");
        return -1;
    }

    //printf("argc = %d\nNOargs + 1 = %d\n", argc, NOargs + 1);

    if (argc == (NOargs + 1) + 1 ) // + file name + flag then we need to check flag
    {
        //printf("here\n");
        if ( strcmp("-d", argv[1]) == 0 )
        {
            fileDump = 1;
        }
        else
        {
            fprintf(stderr, "undefinied flag\n");
            return -1;
        }
    }

    return 0;
} 
