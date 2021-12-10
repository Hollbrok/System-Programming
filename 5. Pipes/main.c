#include "libs.h"
#include "debug.h"

/* Read[0] <- [#####] <- Write[1] */

#define PIPE_R 0 /* constant for more convenient access to pipe read FD*/
#define PIPE_W 1 /* constant for more convenient access to pipe write FD*/

#define B128Kb (1 << 17) /* just constant for designate 128Kb */

#define MIN(a,b) (a) < (b) ? (a) : (b)

volatile sig_atomic_t k_numberOfExitedChilds = 0;
int k_OfChilds;

int totalServed;


/* contains all the information you need for convenient transfer */
struct TransInfo
{
    char *buffer;      /* pointer to allocated bufSize bytes.                                                                           */
    char *endOfBuffer; /* pointer to the 1st byte after buffer                                                                          */ 
    char *readFrom;    /* pointer to the buffer where the read to (child) pipe should come from.                                        */
    char *writeTo;     /* pointer to the buffer where the write from (child) to buffer should take place.                               */

    size_t bufSize;     /* size of transmittion buffer                                                                                  */
    size_t filled;      /* amount of bytes, that are available to be read from us to the 2nd child.                                     */
    
    int RFd;            /* read from pipe to transmission buffer  ; FDs[2 * iTransm][PIPE_R]                                            */
    int WFd;            /* write from transmission buffer to pipe ; FDs[2 * iTransm + 1][PIPE_W]                                        */

    int finished;       /* state of transmission; if finished == 1 => we can close R FD 
                           and if (finished == 1) and filled == 0 => we can close W FD   */
};

/* */

long getNumber(const char *numString, int *errorState);
void handlerSigChld(int signum);

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

    if (nOfChilds < 2 || nOfChilds > 10000)
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

    /* create pipes with O_NONBLOCK! */

    int FDs[2 * (nOfChilds - 1)][2];

    DEBPRINT("Parent PID:      %d\n"
             "Number of child: %d\n"
             "Number of pipes: %d\n", parentPid, nOfChilds, 2 * (nOfChilds - 1));

    for (int i = 0; i < (nOfChilds - 1) * 2; ++i)
        if (pipe2(FDs[i], O_NONBLOCK) == -1)
            err(EX_OSERR, "pipe2");

    errno = 0;

    for (int curChild = 0; curChild < nOfChilds; ++curChild)
    {
        /* preparing */

        int fdR = -1, fdW = -1;

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
  
            /* data transfer */

            int lastRead  = -1;
            char buffer[PIPE_BUF] = {};

            if ( fcntl(fdW, F_SETFL, O_WRONLY) == -1 || fcntl(fdR, F_SETFL, O_RDONLY) == -1 )
                err(EX_OSERR, "~O_NONBLOCK");


            int retSplice = -1;

            while (retSplice != 0)
            {       
                if ((retSplice = splice(fdR, NULL, fdW, NULL, PIPE_BUF, SPLICE_F_MOVE)) == -1)
                    err(EX_OSERR, "splice");

                DEBPRINT("(C%d): transfer %dBytes\n", getpid(), retSplice);
            }
            
            DEBPRINT("(C%d): end of splice\n", getpid());

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
        TI[iTransm].bufSize = MIN(pow(3, nOfChilds - iTransm + 4), B128Kb); //pow(3, nOfChilds - iTransm + 4) > B128Kb ? B128Kb : pow(3, nOfChilds - iTransm + 4);
        
        if ((TI[iTransm].buffer = (char*) calloc(TI[iTransm].bufSize, sizeof(char))) == NULL )
            err(EX_OSERR, "can't calloc memory for buffer");

        TI[iTransm].filled    = 0;
        
        TI[iTransm].endOfBuffer = TI[iTransm].buffer + TI[iTransm].bufSize;

        TI[iTransm].writeTo  = TI[iTransm].buffer;
        TI[iTransm].readFrom = TI[iTransm].buffer;

        TI[iTransm].RFd = FDs[2 * iTransm][PIPE_R];
        TI[iTransm].WFd = FDs[2 * iTransm + 1][PIPE_W];

        TI[iTransm].finished = 0;
    }

    /* data transmission (Parent) */
    
    int endOfTransm = 0;
    totalServed = 0;

    while ((totalServed + k_numberOfExitedChilds) != 2 * nOfTI)
    {
        DEBPRINT("Served %d of %d", totalServed + k_numberOfExitedChilds, 2 * nOfTI);
        fd_set rFds, wFds;

        FD_ZERO(&rFds);
        FD_ZERO(&wFds);

        /* select stuff */
        
        int maxRFd = -1;
        int maxWFd = -1;

        int totalSet = 0;

        for (int iTransm = 0; iTransm < nOfTI; ++iTransm)
        {
            if (!TI[iTransm].finished && (TI[iTransm].bufSize - TI[iTransm].filled /* = amount of empty */) != 0)
            {
                FD_SET(TI[iTransm].RFd, &rFds);
                totalSet++;
                maxRFd = TI[iTransm].RFd > maxRFd ? TI[iTransm].RFd : maxRFd;
            }

            if (TI[iTransm].filled != 0)
            {
                FD_SET(TI[iTransm].WFd, &wFds);
                totalSet++;
                maxWFd = TI[iTransm].WFd > maxWFd ? TI[iTransm].WFd : maxWFd;
            }
        }

        if (totalSet == 0)
            continue;

        int retSelect = -1;

        errno = 0;

        if (maxRFd != -1)
        {   /* R select are possible */
            
        }

        struct timeval zeroT = {0};

        if ((retSelect = select(maxRFd + 1, &rFds, NULL, NULL, &zeroT)) == -1)
        {
            if (errno == EINTR)
            {
                DEBPRINT("EINTR\n");
                continue;
            }   
            else
                err(EX_OSERR, "select (R FDs)");
        }   
        
        if (errno != 0)
            err(EX_OSERR, "P: select read");

        for (int iTransm = 0; iTransm < nOfTI; ++iTransm)
        {
            int readFromPipe  = TI[iTransm].RFd;

            if (FD_ISSET(TI[iTransm].RFd, &rFds)) /* read to us is available for this FD*/
            {
                DEBPRINT("R: iTransm = %d, FD = %d\n", iTransm, TI[iTransm].RFd);

                /* write to buffer from (child) pipe */

                int retRead = -1;
                int writeSize = -1; /* write from pipe to buffer */
                
                /* determine the current state of transmission */

                if (TI[iTransm].writeTo >= TI[iTransm].readFrom)
                {
                                        //  pointer on write to buf --------------\
                                        //                                        V
                                        //      buffer:     [ | | | | | | | | ... | | | ] => we can write up to min(endOfBuf - writeToBuf, PIPE_BUF)
                                        //                             /\
                                        //  pointer on read from buf --/

                    if ((TI[iTransm].writeTo == TI[iTransm].readFrom) && (TI[iTransm].filled == TI[iTransm].bufSize))
                    {
                        /* there are 2 possible situations: filled = 0 or filled = bufSize */
                        /* so if we are filled => can't write to buffer yet                */
                        DEBPRINT("fully filled\n");
                        continue;
                    }

                    writeSize = MIN(PIPE_BUF, TI[iTransm].endOfBuffer - TI[iTransm].writeTo);
                }
                else /* writeTo < readFrom */
                {
                                        //  pointer on write to buf ----\
                                        //                              V
                                        //      buffer:     [ | | | | | | | | ... | | | ] => we can write up to min(readFromBuf - writeToBuf, PIPE_BUF)
                                        //                                        /\
                                        //  pointer on read from buf -------------/
                    
                    writeSize = MIN(PIPE_BUF, TI[iTransm].readFrom - TI[iTransm].writeTo);
                }


                if ((retRead = read(readFromPipe, TI[iTransm].writeTo, writeSize)) == -1)
                    err(EX_OSERR, "read from pipe to transmission buffer");
                
                if (retRead == 0) /* current child has finished transmission*/
                {
                    DEBPRINT("Close R of %d transm\n", iTransm);
                    /* just close unnecessary anymore file descriptor */
                    if (close(TI[iTransm].RFd) == -1)
                        err(EX_OSERR, "close R FD of some child");
                    
                    TI[iTransm].finished = 1;
                    continue;
                }

                /* update transmission info */

                DEBPRINT("read from child pipe %d bytes\n", retRead);

                TI[iTransm].writeTo += retRead;
                TI[iTransm].filled  += retRead;

                DEBPRINT("new filled = %d\n", TI[iTransm].filled);
                
                if (TI[iTransm].writeTo == TI[iTransm].endOfBuffer)
                    TI[iTransm].writeTo = TI[iTransm].buffer;                
            }
        }

        errno = 0;

        if (maxWFd == -1)
        {   /* W select aren't possible */
            continue;
        }

        if ((retSelect = select(maxWFd + 1, NULL, &wFds, NULL, &zeroT)) == -1)
        {
            if (errno == EINTR)
            {
                DEBPRINT("W: EINTR\n");
                continue;
            }   
            else
                err(EX_OSERR, "select (W FDs)");
        }   
        
        if (errno != 0)
            err(EX_OSERR, "P: select write");

        if (retSelect == 0)
        {
            DEBPRINT("Test: (retSelect == 0) (W)\n");
            continue;
        }

        for (int iTransm = 0; iTransm < nOfTI; ++iTransm)
        {
            int writeToPipe   = TI[iTransm].WFd;

            if (FD_ISSET(TI[iTransm].WFd, &wFds)) /* write from us to child is available for this FD */
            {
                DEBPRINT("W: iTransm = %d, FD = %d\n", iTransm, TI[iTransm].WFd);

                int retWrite = -1;
                int readSize = -1; /* read from buffer to pipe */

                /* determine the current state of transmission */

                if (TI[iTransm].writeTo >= TI[iTransm].readFrom)
                {
                                        //  pointer on write to buf --------------\
                                        //                                        V
                                        //      buffer:     [ | | | | | | | | ... | | | ] => we can write up to min(writeToBuf - readFromBuf, PIPE_BUF)
                                        //                             /\
                                        //  pointer on read from buf --/

                    if ((TI[iTransm].writeTo == TI[iTransm].readFrom) && (TI[iTransm].filled == 0))
                    {
                        /* there are 2 possible situations: filled = 0 or filled = bufSize */
                        /* so if we are empty => can't write to buffer yet                */
                        continue;
                    }

                    if ((TI[iTransm].writeTo == TI[iTransm].readFrom) && (TI[iTransm].filled == TI[iTransm].bufSize)) // full filled
                        readSize = MIN(PIPE_BUF, TI[iTransm].endOfBuffer - TI[iTransm].readFrom);
                    else if (TI[iTransm].writeTo > TI[iTransm].readFrom)
                        readSize = MIN(PIPE_BUF, TI[iTransm].writeTo - TI[iTransm].readFrom);
                    else
                    {
                        fprintf(stderr, "LOGIC ERROR #2\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else /* writeTo < readFrom */
                {
                                        //  pointer on write to buf ----\
                                        //                              V
                                        //      buffer:     [ | | | | | | | | ... | | | ] => we can write up to min(endOfBuf - readFromBuf, PIPE_BUF)
                                        //                                        /\
                                        //  pointer on read from buf -------------/
                    
                    readSize = MIN(PIPE_BUF, TI[iTransm].endOfBuffer - TI[iTransm].readFrom);
                }

                /* if between select and write one of the children dies => this call'll cause SIGPIPE + EPIPE */
                if ((retWrite = write(writeToPipe, TI[iTransm].readFrom, readSize)) == -1)  
                    err(EX_OSERR, "write to pipe from transmission buffer");

                /* update transmission info */

                TI[iTransm].readFrom += retWrite;

                TI[iTransm].filled   -= retWrite;
                
                if (TI[iTransm].readFrom == TI[iTransm].endOfBuffer)
                {
                    DEBPRINT("readFrom == end (transfer it to the beginning of buffer)\n");
                    TI[iTransm].readFrom = TI[iTransm].buffer;
                }

                if (TI[iTransm].readFrom > TI[iTransm].endOfBuffer)
                {
                    DEBPRINT("LOGIC ERROR, OUT OF BUFFER\nreadFrom - endBuffer = %d\n", TI[iTransm].readFrom - TI[iTransm].endOfBuffer);
                    exit(EXIT_FAILURE);
                }

                if (TI[iTransm].finished && TI[iTransm].filled == 0)
                {
                    if (close(TI[iTransm].WFd) == -1)
                        err(EX_OSERR, "close W FD of some child");

                    totalServed++;
                }
            }
        }
    
        if (endOfTransm)
        {
            DEBPRINT("END OF TRANSM\n");
            break;
        }
    }
    
    fprintf(stderr, "P: FINISHED\n");

    /* waiting for end of transmission */

    while (wait(NULL) != -1)
    {}

    fprintf(stderr, "Success\n");

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
        //totalServed++;

        if (k_numberOfExitedChilds == k_OfChilds)
        {
            fprintf(stderr, "All childs are served\n");
            exit(EXIT_SUCCESS);
        }

        return;
    }

    DEBPRINT("\t some child didn't exit or exited, but with no EXIT_SUCCESS\n");

    return;//exit(EXIT_FAILURE);
}