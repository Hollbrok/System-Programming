#include "libs.h"
#include "commun.h"


static int getRDofFIFO();

static int getRDofFIFOAccess();
 
static void handlerFIFO(int sig);

static void setSignalsHandler();

static void fixFifoEof();

static void createServerFIFO();

static void createServerFIFOAccess();

int main(int argc, const char *argv[])
{
    int serverRFd = -1;             /* read client request              */
    int serverAccRFd = -1;          /* read client accessing request    */
    int clientWFd = -1;             /* write server response            */

    struct Req req;        /* request to server from client     */
    struct AccReq accReq;
    struct Accresp accResp = {1};

    char clientFifo[CLIENT_FIFO_NAME_LEN];


/* Set signals (INT + TERM) handlers                            */

    setSignalsHandler();

/* Creating FIFO (only 1 for server to read from all clients)   */

    createServerFIFO();

/* Creating Access FIFO (only 1 for server to read from all clients)   */

    createServerFIFOAccess();

/* Get FD to read from serverAccessFIFO                               */
 
    serverAccRFd = getRDofFIFOAccess();


/* fix that fifo may meet EOF if there are no clients           */

    fixFifoEof();


    /*if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        fprintf(stderr, "Can't set ignore to signal SISPIPE.\n");
        exit(SIGPIPE_IGN_ERROR);
    }*/

/* getting requests from clients                           */

    fprintf(stderr, "TEST: Before while(1)\n");

    while (TRUE)
    {
        /* trying to READ REQUEST from client until we got it                                           */

        if (read(serverAccRFd, &accReq, sizeof(struct AccReq)) != sizeof(struct AccReq))
        {
            //fprintf(stderr, "Not full read from client [ACCESS request] or error, continue to read.\n");
            continue;
        }

        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) accReq.pid); 

        int clientRFd = open(clientFifo, O_WRONLY);
        if (clientRFd < 0)
        {
            fprintf(stderr, "clientFifo = %s\n", clientFifo);
            perror("clientFifo");
        }

        // may be sleep(5)?
        // sleep(5);

        if (write(clientRFd, &accResp, sizeof(struct Accresp)) != sizeof(struct Accresp))
        {
            perror("answering to client [write clietFIFO]");
            continue;
        }

        
        int lastByteRead;
        errno = 0;

        serverRFd = getRDofFIFO();

        while ( (lastByteRead = read(serverRFd, &req, BUF_SIZE)) > 0) // == BUF_SIZE
        {
            //fprintf(stderr, "[%d]\n", lastByteRead);
            fprintf(stderr, "%.*s", lastByteRead, req.buffer);
        }

        if (lastByteRead != 0)
        {
            perror("reader\n");
            exit(EXIT_FAILURE);
        }

        fprintf(stderr, "CLIENT SERVED\n");

    }

// close all FD
    exit(EXIT_SUCCESS);
}

static void handlerFIFO(int sig)
{
    unlink(SERVER_FIFO);
    unlink(SERVER_FIFO_ACCESS);

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
    //int fixFd  = -1;    /* this is FD to fix problem with FIFO: it needs to open FIFO write end to not meet EOF. */

    /*fixFd = open(SERVER_FIFO, O_WRONLY);

    if (fixFd  == -1)
    {
        fprintf(stderr, "ERROR: open fixFD. %s\n", SERVER_FIFO);
        exit(OPEN_FIX_FD);
    }*/

    int fixAccFd  = -1;    /* this is FD to fix problem with FIFO: it needs to open FIFO write end to not meet EOF. */

    fixAccFd = open(SERVER_FIFO_ACCESS, O_WRONLY);

    if (fixAccFd  == -1)
    {
        fprintf(stderr, "ERROR: open fixFD. %s\n", SERVER_FIFO_ACCESS);
        exit(OPEN_FIX_FD);
    }
}

static void createServerFIFO()
{
    umask(0);           
    errno = 0;

    int mkfifoStatus = mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP);

    /*while (mkfifoStatus == -1)
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
    }*/

    if (mkfifoStatus == -1)
    {
        if(errno != EEXIST)
        {
            perror("ERROR: mk(client)fifo. ERROR IS NOT EEXIST\n");
            exit(MKFIFO_NO_EEXIT);
        }
    }  
}

static void createServerFIFOAccess()
{
    umask(0);           
    errno = 0;

    int mkfifoStatus = mkfifo(SERVER_FIFO_ACCESS, S_IRUSR | S_IWUSR | S_IWGRP);

    /*while (mkfifoStatus == -1)
    {
        perror("TEST: MKFIFO returns -1\nTEST: errno");

        if (unlink(SERVER_FIFO_ACCESS) == -1)
        {
            perror("Can't unlink FIFO, exit\n");
            exit(EXIT_FAILURE);
        }

        if (errno != EEXIST)
        {
            fprintf(stderr, "ERROR: mk(client)fifo. ERROR IS NOT EEXIST\n");
            exit(MKFIFO_NO_EEXIT);
        } 

        mkfifoStatus = mkfifo(SERVER_FIFO_ACCESS, S_IRUSR | S_IWUSR | S_IWGRP);
    }*/

    if (mkfifoStatus == -1)
    {
        if(errno != EEXIST)
        {
            perror("ERROR: mk(client)fifo access. ERROR IS NOT EEXIST\n");
            exit(MKFIFO_NO_EEXIT);
        }
    }  
}

static int getRDofFIFO()
{
    int serverRFd = open(SERVER_FIFO, O_RDONLY); //| O_NONBLOCK);  
    if (serverRFd == -1)
    {
        fprintf(stderr, "ERRORopen %s", SERVER_FIFO);
        exit(ERROR_OPEN_TO_READ_CLIENT);
    }

    return serverRFd;
}

static int getRDofFIFOAccess()
{
    int serverRFd = open(SERVER_FIFO_ACCESS, O_RDONLY | O_NONBLOCK);
    if (serverRFd == -1)
    {
        fprintf(stderr, "ERRORopen %s", SERVER_FIFO_ACCESS);
        exit(ERROR_OPEN_TO_READ_CLIENT);
    }

    return serverRFd;
}