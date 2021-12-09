#include "libs.h"
#include "debug.h"

/* Read[0] <- [#####] <- Write[1] */

#define PIPE_R 0 /* constant for more convenient access to pipe read FD*/
#define PIPE_W 1 /* constant for more convenient access to pipe write FD*/

#define B128Kb (1 << 17) /* just constant for designate 128Kb */

volatile sig_atomic_t k_numberOfExitedChilds = 0;
int k_OfChilds;


/* contains all the information you need for convenient transfer */
struct TransInfo
{
    char *buffer;       /* pointer to allocated bufSize bytes.                                                                          */
    char *read_from;    /* pointer in the buffer where the read to (child) pipe should come from.                                       */
    char *write_to;     /* pointer in the buffer where the write from (child) to buffer should take place.                              */

    size_t bufSize;     /* size of transmittion buffer                                                                                  */
    size_t filled;      /* amount of bytes, that are available to be read from us to the 2nd child.                                     */
    size_t empty;       /* (bufSize - filled); So amount of bytes, that are available to be written to us from the 1st child.           */
    
    int RFd;            /* FDs[2 * iTransm][PIPE_R]                                                                                     */
    int Wfd;            /* FDs[2 * iTransm + 1][PIPE_W]                                                                                 */
};

/* */

long getNumber(const char *numString, int *errorState);

/*  argv[1] -- number of childs 
    argv[2] -- file-name        */

int main(int argc, const char *argv[])
{
    if (argc != 3)
        err(EX_USAGE, "program needs number and file argument");

    int errorNumber = -1;
    int nOfChilds   = k_OfChilds = getNumber(argv[1], &errorNumber);

    if (errorNumber == -1)
        err(EX_USAGE, "not-a-number in argument");

    if (2 > nOfChilds || nOfChilds > 10000)
        err(EX_USAGE, "number should be from 2 to 10000");  

    /* fork initialization (with creating pipes, setting signal handlers, closing FDs, etc.) */

    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sa.sa_handler = handlerSigChld;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        err(EX_OSERR, "sigaction to SIGCHLD");

    /* */

    int isParent = -1;
    int parentPid = getpid();

    fprintf(stderr, "Parent PID: %ld\n", (long)getpid());


    /* create pipes with O_NONBLOCK! */

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

        // можно динамически от ребенка к ребенку создавать pipe, а закрывать соответственно все предыдущие.

        /*  */

        switch (isParent = fork())
        {
        case -1:    /*   ERROR   */
            err(EX_OSERR, "fork");
            break;
        case 0:     /*   CHILD   */
        {
        
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

            //DEBPRINT("\n\t Child before R/W\n");
            
            while (lastRead != 0)
            {
                if ((lastRead = read(fdR, buffer, PIPE_BUF)) == -1)
                    err(EX_OSERR, "C: read");
                if (write(fdW, buffer, lastRead) == -1)
                    err(EX_OSERR, "C: write");

                //DEBPRINT("after read-write [lbr = %d]\n", lastRead);
            }
            

            if (close(fdR) == -1 || close(fdW) == -1)
                err(EX_OSERR, "close fdR/fdW");

            DEBPRINT("\tSuccess\n");
            exit(EXIT_SUCCESS);
            break;
        }
        default:    /*   PARENT  */

            break;
        }

    }

    /* close FDs that won't be used */

    if (close(FDs[0][PIPE_W]) == -1 || close(FDs[(nOfChilds - 1) * 2 - 1][PIPE_R]) == -1)
        err(EX_OSERR, "P: close W/R 0/end-child");

    for (int iChild = 1; iChild < nOfChilds - 1; ++iChild)
    {
        if (close(FDs[iChild * 2 - 1][PIPE_R]) == -1 || close(FDs[iChild * 2][PIPE_W]) == -1)
            err(EX_OSERR, "P: close R/W of pipes");
    }

    /* allocate transmission structres */

    int nOfTI = nOfChilds - 1;

    struct TransInfo *TI = (struct TransInfo*) (calloc(nOfTI, sizeof(struct TransInfo)));
    if (TI == NULL)
        err(EX_OSERR, "Can't calloc info for TransInfo structure");
    
    /* allocate space for TI buffers (+ calculate the bufSize)*/

    for (int iTransm = 0; iTransm < nOfTI; ++iTransm)
    {
        TI[iTransm].bufSize = pow(3, nOfChilds - iTransm + 4) > B128Kb ? B128Kb : pow(3, nOfChilds - iTransm + 4);
        
        if ((TI[iTransm].buffer = (char*) calloc(TI[iTransm].bufSize, sizeof(char))) == NULL )
            err(EX_OSERR, "can't calloc memory for buffer");

        TI[iTransm].filled    = 0;
        TI[iTransm].empty     = TI[iTransm].bufSize;
        
        TI[iTransm].write_to  = TI[iTransm].buffer;
        TI[iTransm].read_from = TI[iTransm].buffer;

        TI[iTransm].RFd = FDs[2 * iTransm][PIPE_R];
        TI[iTransm].Wfd = FDs[2 * iTransm + 1][PIPE_W];
    }

    /* data transmission (Parent) */

    int endOfTransm = 0;

    while (1)
    {
        fd_set rFds, wFds;

        FD_ZERO(&rFds);
        FD_ZERO(&wFds);

        /* select stuff */
        
        int maxRFd = -1;
        int maxWFd = -1;

        for (int iTransm = 0; iTransm < nOfTI; ++iTransm)
        {
            FD_SET(TI[iTransm].RFd, &rFds);
            FD_SET(TI[iTransm].Wfd, &wFds);
        
            maxRFd = TI[iTransm].RFd > maxRFd ? TI[iTransm].RFd : maxRFd;
            maxWFd = TI[iTransm].Wfd > maxWFd ? TI[iTransm].Wfd : maxWFd;
        }

        int retSelect = -1;

        errno = 0;

        if ((retSelect = select(maxRFd + 1, &rFds, NULL, NULL, NULL)) == -1)
            err(EX_OSERR, "select (R FDs)");
        
        if (errno != 0)
            err(EX_OSERR, "P: select read");

        if (retSelect == 0)
        {
            fprintf(stderr, "Test: (retSelect == 0)\n");
            continue;
        }

        for (int iTransm = 0; iTransm < nOfTI; ++iTransm)
        {
            if (FD_ISSET(TI[iTransm].RFd, &rFds)) /* read from this FD is available */
            {
                // write to buffer from (child) pipe

            }

        }


        for (int iTransm = 0; iTransm < nOfChilds - 1; ++iTransm)
        {
            int lastRead = -1;

            if (FD_ISSET(FDs[2 * iTransm][PIPE_R], &rFds))
                if ((lastRead = read(FDs[2 * iTransm][PIPE_R], buffer, readSize)) == -1)
                    err(EX_OSERR, "P: read in cycle");

            if (lastRead == 0) /* most probably unnecessary check */
            {
                DEBPRINT("find EOF, let's new transm\n");
                endOfTransm = 1;
                break;
            }

            while (select(FDs[2 * iTransm + 1][PIPE_W] + 1, NULL, &wFds, NULL, NULL) == 0) // last NULL ===> &{0}
            {}


            if (errno != 0)
                err(EX_OSERR, "P: select write");
        
            if (FD_ISSET(FDs[2 * iTransm + 1][PIPE_W], &wFds))
                if (write(FDs[2 * iTransm + 1][PIPE_W], buffer, lastRead) != lastRead)
                    err(EX_OSERR, "P: write in cycle");

            //free(buffer);

            DEBPRINT("\n\t PARENT TRANSM: %dbyte to %d child\n", lastRead, iTransm + 1); 

        }
    
        if (endOfTransm)
        {
            for (int iTransm = 0; iTransm < nOfChilds - 1; ++iTransm)
                if (close(FDs[2 * iTransm][PIPE_R]) == -1 || close(FDs[2 * iTransm + 1][PIPE_W]) == -1)
                    err(EX_OSERR, "P: close PROCESSES FDs");

            DEBPRINT("END OF TRANSM\n");
            break;
        }
    }

    /*for (int iTransm = 0; iTransm < nOfChilds - 1; ++iTransm)
    {
        DEBPRINT("Lets do transm #%d\n", iTransm);

        int bufferSize = pow(3, nOfChilds - iTransm + 4) < (1 << 17) ? pow(3, nOfChilds - iTransm + 4) : (1 << 17);

        int readSize   = bufferSize < PIPE_BUF ? bufferSize : PIPE_BUF;
        int lastRead   = -1;

        char *buffer = (char*) calloc(bufferSize, sizeof(char));
        if (buffer == NULL)
            err(EX_OSERR, "can't calloc");

        DEBPRINT("calloc buffer size[i = %d] = %d\n", iTransm, bufferSize);

        fd_set rFds, wFds;
        
        FD_ZERO(&rFds);
        FD_ZERO(&wFds);

        FD_SET(FDs[2 * iTransm][PIPE_R], &rFds);
        FD_SET(FDs[2 * iTransm + 1][PIPE_W], &wFds);

        errno = 0;

        while (1) // lastRead != 0
        {
            //read

            DEBPRINT("before select R (%d)\n", FDs[2 * iTransm][PIPE_R]);

            while (select(FDs[2 * iTransm][PIPE_R] + 1, &rFds, NULL, NULL, NULL) == 0) // last NULL ===> &{0}
            {}

            DEBPRINT("after select R\n");

            if (errno != 0)
                err(EX_OSERR, "P: select read");
        
            if (FD_ISSET(FDs[2 * iTransm][PIPE_R], &rFds))
                if ((lastRead = read(FDs[2 * iTransm][PIPE_R], buffer, readSize)) == -1)
                    err(EX_OSERR, "P: read in cycle");

            if (lastRead == 0)
            {
                DEBPRINT("find EOF, let's new transm\n");

                if (close(FDs[2 * iTransm][PIPE_R]) == -1 || close(FDs[2 * iTransm + 1][PIPE_W]) == -1)
                    err(EX_OSERR, "P: close PROCESSES FDs");

                break;
            }

            // write

            DEBPRINT("before select W\n");

            while (select(FDs[2 * iTransm + 1][PIPE_W] + 1, NULL, &wFds, NULL, NULL) == 0) // last NULL ===> &{0}
            {}

            DEBPRINT("after select W\n");


            if (errno != 0)
                err(EX_OSERR, "P: select write");
        
            if (FD_ISSET(FDs[2 * iTransm + 1][PIPE_W], &wFds))
                if (write(FDs[2 * iTransm + 1][PIPE_W], buffer, lastRead) != lastRead)
                    err(EX_OSERR, "P: write in cycle");

            fprintf(stderr, "\n\t PARENT TRANSM: %dbyte to %d child\n", lastRead, iTransm + 1);
        }

        free(buffer);
    } */
    
    /* waiting for end */

    DEBPRINT("\n\tSuccess, waiting all childs...\n");

    while (wait(NULL) != -1)
    {}

    DEBPRINT("\n\tAll childs have finished\n");

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


void handlerSigChld(int signum)
{
    DEBPRINT("One of the children died\n");

    int status = -1;

    wait(&status);

    if (WIFEXITED(status) && (WEXITSTATUS(status) == EXIT_SUCCESS) )
    {
        DEBPRINT("\t some child exited with EXIT_SUCCESS\n");
        
        k_numberOfExitedChilds++;

        if (k_numberOfExitedChilds == k_OfChilds)
            exit(EXIT_SUCCESS);
        
        return;
    }

    DEBPRINT("\t some child didn't exit or exited, but with no EXIT_SUCCESS\n");

    exit(EXIT_FAILURE);
}