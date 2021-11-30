#include "libs.h"
#include "debug.h"
#include "common.h"

/* Read[0] <- [#####] <- Write[1] */

#define PIPE_R 0
#define PIPE_W 1

/* */

long getNumber(const char *numString, int *errorState);

/*  argv[1] -- number of childs 
    argv[2] -- file-name        */

int main(int argc, const char *argv[])
{
    if (argc < 2)
        err(EX_USAGE, "program needs number argument");

    int errorNumber = -1;
    int nOfChilds = getNumber(argv[1], &errorNumber);

    if (errorNumber == -1)
        err(EX_USAGE, "not-a-number in argument");

    if (2 > nOfChilds || nOfChilds > 10000)
        err(EX_USAGE, "number should be from 2 to 10000");  

    /* fork initialization (with creating pipes, closing FDs, etc.) */

    int isParent = -1;
    int parentPid = getpid();

    fprintf(stderr, "Parent PID: %ld\n", (long)getpid());

    int FDs[2 * (nOfChilds - 1)][2];

    fprintf(stderr, "Number of pipes: %d\n", 2 * (nOfChilds - 1));

    for (int i = 0; i < (nOfChilds - 1) * 2; ++i)
    {
        if (pipe2(FDs[i], O_NONBLOCK) == -1)
            err(EX_OSERR, "pipe2");
    }


    errno = 0;

    for (int curChild = 0; curChild < nOfChilds; ++curChild)
    {
        /* preparing */

        /*  */

        switch (isParent = fork())
        {
        case -1:    /*   ERROR   */
            err(EX_OSERR, "fork");
            break;
        case 0:     /*   CHILD   */
        
            //fprintf(stderr, "Child %ld\n", (long)getpid());

            if (prctl(PR_SET_PDEATHSIG, SIGHUP) == -1)
                err(EX_OSERR, "prctl");
        
            if (parentPid != getppid())
                err(EX_OSERR, "err pid");

        /* close not interesting for this child FDs*/

            /* 0-child */

            if (curChild == 0)
            {
                if (close(FDs[0][PIPE_R]) == -1)
                    err(EX_OSERR, "close 0-child read-end");
            
                for (int i = 1; i < (nOfChilds - 1) * 2; ++i)
                    if (close(FDs[i][PIPE_R]) == -1 || close(FDs[i][PIPE_W]) == -1)
                        err(EX_OSERR, "close pipe fds for 0-child");
            }
            /* last child*/

            else if (curChild == nOfChilds - 1)
            {
                if (close(FDs[(nOfChilds - 1) * 2 - 1][PIPE_W]) == -1)
                    err(EX_OSERR, "close last-child write-end");

                for (int i = 0; i < (nOfChilds - 1) * 2 - 1; ++i)
                    if (close(FDs[i][PIPE_R]) == -1 || close(FDs[i][PIPE_W]) == -1)
                        err(EX_OSERR, "close pipe fds for end-child");
            }

            /* anothers */

            else
            {
                if (close(FDs[curChild * 2 - 1][PIPE_W]) == -1 || close(FDs[curChild * 2][PIPE_R]) == -1)
                    err(EX_OSERR, "close W/R pairs respectively");

                for (int i = 0; i < (nOfChilds - 1) * 2; ++i)
                    if (i == curChild * 2 -1)
                        ++i;
                    else if (close(FDs[i][PIPE_R]) == -1 || close(FDs[i][PIPE_W]) == -1)
                        err(EX_OSERR, "close pipe fds for 0-child");

            }
            /* end of close FDs that we are not interested in */
  
            fprintf(stderr, "%ld: SUCCESS\n", (long)getpid());
            exit(EXIT_SUCCESS);
            break;
        default:    /*   PARENT  */

            break;
        }

    }

    /* data transmission  */




    fprintf(stderr, "%ld: SUCCESS\n", (long)getpid());
    exit(EXIT_SUCCESS);
}


long getNumber(const char *numString, int *errorState)
{
    *errorState = 0;

    if (*numString == '\0')
    {
        fprintf(stderr, "empty number argument\n");
        *errorState = -1;
        return 0;
    }

    errno = 0;

    long gNumber;
    char* endOfEnter;

    const int baseOfNumber = 10;
    gNumber = strtol(numString, &endOfEnter, baseOfNumber);

    if(*endOfEnter != '\0')
    {
        fprintf(stderr, "strtol error\n");
        *errorState = -1;
    }
    if (errno != 0)
    {
        fprintf(stderr, "strtol error\n");
        *errorState = -1;
    }
    
    return gNumber;

}