#include "libs.h"

static const int BUF_SIZE = 4096;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "NP arguments must be equal 2\n");
        exit(EXIT_FAILURE);
    }

    char buffer[BUF_SIZE] = {};

    int FDs[2];

    /*
        FDs[0] -- read
        FDs[1] -- write    
                        */

    if( pipe(FDs) != 0)
    {
        perror("Error in pipe creating\n");
        exit(EXIT_FAILURE);
    }

    switch (fork())
    {
        case -1:    /* ERROR in fork() */
            perror("fork()\n");

            if (close(FDs[0]) == -1)
            {
                perror("Close FDs[0](error in fork)");
                exit(EXIT_FAILURE);
            }

            if (close(FDs[1]) == -1)
            {
                perror("Close FDs[1](error in fork)");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_FAILURE);
            break;
        case 0:     /* CHILD */

            if (close(FDs[0]) == -1)
            {
                perror("Close FDs[0] in child");
                exit(EXIT_FAILURE);
            }

            int fileD;

            if ( (fileD = open(argv[1], O_RDONLY)) == -1)
            {
                perror("open argv[1]");
                exit(EXIT_FAILURE);
            }

            int lastByteReadC;

            while ( (lastByteReadC = read(fileD, buffer, BUF_SIZE)) > 0 )
            {
                if ( write(FDs[1], buffer, lastByteReadC) != lastByteReadC)
                {
                    perror("write to FDs[1]");
                    exit(EXIT_FAILURE);
                }
            }   

            if (close(FDs[1]) == -1)
            {
                perror("Close FDs[1] in child");
                exit(EXIT_FAILURE);
            }

            break;
        default:    /* PARENT */

            if (close(FDs[1]) == -1)
            {
                perror("Close FDs[1] in parent");
                exit(EXIT_FAILURE);
            }
            
            int lastByteReadP;
            while ( (lastByteReadP = read(FDs[0], buffer, BUF_SIZE)) > 0 )
            {
                fprintf(stderr, "%.*s", lastByteReadP, buffer);
            }

            if (close(FDs[0]) == -1)
            {
                perror("Close FDs[0] in parent");
                exit(EXIT_FAILURE);
            }
    }

    exit(EXIT_SUCCESS);
}

