#include "libs.h"
#include "commun.h"


static int getRDofFIFO();
 
static void handlerFIFO(int sig);

static void setSignalsHandler();

static void fixFifoEof();

static void createServerFIFO();

int main(int argc, const char *argv[])
{
    int serverRFd = -1;             /* read client request              */
    int clientWFd = -1;             /* write server response            */

    struct Req req;        /* request to server from client    */


/* Set signals (INT + TERM) handlers                            */

    setSignalsHandler();

/* Creating FIFO (only 1 for server to read from all clients)   */

    createServerFIFO();

/* Get FD to read from serverFIFO                               */
 
    serverRFd = getRDofFIFO();

/* fix that fifo may meet EOF if there are no clients           */

    fixFifoEof();


    /*if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        fprintf(stderr, "Can't set ignore to signal SISPIPE.\n");
        exit(SIGPIPE_IGN_ERROR);
    }*/

/* getting requests from clients                           */

    pid_t curClient = -1;

    while (TRUE)
    {
        /* trying to READ REQUEST from client until we got it                                           */

        int lastByteRead;
        int NOaccess = 0;

        while ( (lastByteRead = read(serverRFd, &req, BUF_SIZE + 2 * sizeof(int) + sizeof(pid_t))) > 0) // == BUF_SIZE
        {
            //fprintf(stderr, "TEST: lastByteRead = %d\n", lastByteRead);
            if (curClient == -1)
                curClient = req.pid;
            
            if (req.pid == curClient)
            {
                if (req.NOaccess == NOaccess)
                {
                    fprintf(stderr, "%.*s", req.realSize, req.buffer);
                    ++NOaccess;
                }
                else
                    fprintf(stderr, "Cur Process[%d] does not transfer data from the beginning of the file (NOaccess = %d)\n", req.pid, req.NOaccess);
            }

            /* DEBUG */
            else
            { 
                fprintf(stderr, "maximum number of clients at a time = 1");
                continue;
            }
        }

        curClient = -1;
    }

    exit(EXIT_SUCCESS);
}

static void handlerFIFO(int sig)
{
    unlink(SERVER_FIFO);

    fprintf(stderr, "FIFO handler got SIGNAL: %s(%d)\n", strsignal(sig), sig);

    exit(FIFO_HANDLER_SIGNALS);
}

static void setSignalsHandler()
{
    if ( (signal(SIGINT, handlerFIFO) == SIG_ERR) || (signal(SIGTERM, handlerFIFO) == SIG_ERR) )
    {
        fprintf(stderr, "Can't set signal handler for SIGINT or SIGTERM\n");
        exit(SET_SIGHANDLER_ERROR);
    }
}

static void fixFifoEof()
{
    int fixFd  = -1;    /* this is FD to fix problem with FIFO: it needs to open FIFO write end to not meet EOF. */

    fixFd = open(SERVER_FIFO, O_WRONLY);

    if (fixFd  == -1)
    {
        fprintf(stderr, "ERROR: open fixFD. %s\n", SERVER_FIFO);
        exit(OPEN_FIX_FD);
    }
}

static void createServerFIFO()
{
    umask(0);           
    errno = 0;

    int mkfifoStatus = mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP);

    while (mkfifoStatus == -1)
    {
        perror("TEST: MKFIFO returns -1\nTEST: errno");

        if (unlink(SERVER_FIFO) == -1)
        {
            perror("Can't unlink FIFO, exit\n");
            exit(EXIT_FAILURE);
        }

        if (errno != EEXIST)
        {
            fprintf(stderr, "ERROR: mk(client)fifo. ERROR IS NOT EEXIST\n");
            exit(MKFIFO_NO_EEXIT);
        } 

        mkfifoStatus = mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP);
    }

}

static int getRDofFIFO()
{
    int serverRFd = open(SERVER_FIFO, O_RDONLY |  O_NONBLOCK);//O_RDONLY);
    if (serverRFd == -1)
    {
        fprintf(stderr, "ERRORopen %s", SERVER_FIFO);
        exit(ERROR_OPEN_TO_READ_CLIENT);
    }

    return serverRFd;
}