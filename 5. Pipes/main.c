#include "libs.h"
#include "debug.h"

/* Read[0] <- [#####] <- Write[1] */

#define PIPE_R 0
#define PIPE_W 1

/* */

long getNumber(const char *numString, int *errorState);

/*  argv[1] -- number of childs 
    argv[2] -- file-name        */

int main(int argc, const char *argv[])
{
    if (argc != 3)
        err(EX_USAGE, "program needs number and file argument");

    int errorNumber = -1;
    int nOfChilds = getNumber(argv[1], &errorNumber);

    if (errorNumber == -1)
        err(EX_USAGE, "not-a-number in argument");

    if (2 > nOfChilds || nOfChilds > 10000)
        err(EX_USAGE, "number should be from 2 to 10000");  

    /* fork initialization (with creating pipes, closing FDs, etc.) */

    /* */

    // SIGCHILD

    /* */

    int isParent = -1;
    int parentPid = getpid();

    fprintf(stderr, "Parent PID: %ld\n", (long)getpid());

    int FDs[2 * (nOfChilds - 1)][2];

    fprintf(stderr, "Number of pipes: %d\n", 2 * (nOfChilds - 1));

    for (int i = 0; i < (nOfChilds - 1) * 2; ++i)
        if (pipe2(FDs[i], O_NONBLOCK) == -1)
            err(EX_OSERR, "pipe2");

    fprintf(stderr, "\n");

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
        {
        
            //fprintf(stderr, "Child %ld\n", (long)getpid());

            if (prctl(PR_SET_PDEATHSIG, SIGHUP) == -1)
                err(EX_OSERR, "prctl");
        
            if (parentPid != getppid())
                err(EX_OSERR, "err ppid");

            /* close FDs that we are not interested in*/

            int fdR = -1, fdW = -1;

            /* 0-child */

            if (curChild == 0)
            {
                if (close(FDs[0][PIPE_R]) == -1)
                    err(EX_OSERR, "close 0-child read-end");
            
                for (int i = 1; i < (nOfChilds - 1) * 2; ++i)
                    if (close(FDs[i][PIPE_R]) == -1 || close(FDs[i][PIPE_W]) == -1)
                        err(EX_OSERR, "close pipe fds for 0-child");

                if ((fdR = open(argv[2], O_RDONLY)) == -1)
                    err(EX_OSERR, "open file on read");

                fdW = FDs[0][PIPE_W];
            }
            /* last child*/

            else if (curChild == nOfChilds - 1)
            {
                fprintf(stderr, "\n\t%ld: Last Child\n"
                                "check FD = %d\n", (long)getpid(), (nOfChilds - 1) * 2 - 1);

                if (close(FDs[(nOfChilds - 1) * 2 - 1][PIPE_W]) == -1)
                    err(EX_OSERR, "close last-child write-end");

                for (int i = 0; i < (nOfChilds - 1) * 2 - 1; ++i)
                    if (close(FDs[i][PIPE_R]) == -1 || close(FDs[i][PIPE_W]) == -1)
                        err(EX_OSERR, "close pipe fds for end-child");

                fdR = FDs[(nOfChilds - 1) * 2 - 1][PIPE_R];
                fdW = STDOUT_FILENO;
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

                fdR = FDs[curChild * 2 - 1][PIPE_R];
                fdW = FDs[curChild * 2][PIPE_W];
            }

            /* end of close FDs that we are not interested in */
  
            /* data transfer */

            int lastRead  = -1;
            char buffer[PIPE_BUF] = {};

            if ( fcntl(fdW, F_SETFL, O_WRONLY) == -1 ||
                 fcntl(fdR, F_SETFL, O_RDONLY) == -1 )
                err(EX_OSERR, "~O_NONBLOCK");

            
            while (lastRead != 0)
            {
                if ((lastRead = read(fdR, buffer, PIPE_BUF)) == -1)
                    err(EX_OSERR, "C: read");
                if (write(fdW, buffer, lastRead) == -1)
                    err(EX_OSERR, "C: write");

                DEBPRINT("C: after read-write [lbr = %d]\n", lastRead);
            }
            

            if (close(fdR) == -1 || close(fdW) == -1)
                err(EX_OSERR, "close fdR/fdW");

            fprintf(stderr, "\n\t%ld: Success\n", (long)getpid());
            exit(EXIT_SUCCESS);
            break;
        }
        default:    /*   PARENT  */

            break;
        }

    }

    /* close unused FDs */

    if (close(FDs[0][PIPE_W]) == -1 || close(FDs[(nOfChilds - 1) * 2 - 1][PIPE_R]) == -1)
        err(EX_OSERR, "P: close W/R 0/end-child");

    for (int iChild = 1; iChild < nOfChilds - 1; ++iChild)
    {
        if (close(FDs[iChild * 2 - 1][PIPE_R]) == -1 || close(FDs[iChild * 2][PIPE_W]) == -1)
            err(EX_OSERR, "P: close R/W of pipes");
    }

    /* data transmission (Parent) */

    for (int iTransm = 0; iTransm < nOfChilds - 1; ++iTransm)
    {
        int bufferSize = pow(3, nOfChilds - iTransm + 4) < (1 << 17) ? pow(3, nOfChilds - iTransm + 4) : (1 << 17);

        char *buffer = (char*) calloc(bufferSize, sizeof(char));
        if (buffer == NULL)
            err(EX_OSERR, "can't calloc");

        DEBPRINT("calloc buffer size[i = %d] = %d\n", iTransm, bufferSize);


        /*fd_set rFds, wFds;
        
        FD_ZERO(&rFds);
        FD_ZERO(&wFds);

        FD_SET(FDs[2 * iTransm][PIPE_R], &rFds);
        FD_SET(FDs[2 * iTransm + 1][PIPE_W], &wFds); */

    
        free(buffer);
    } 

    /* */

    fprintf(stderr, "\n\t(P) %ld: SUCCESS\n", (long)getpid());
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