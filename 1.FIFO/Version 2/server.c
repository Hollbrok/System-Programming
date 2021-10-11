#include "libs.h"
#include "commun.h"
 
static int NEED_UNLINK_SAFIFO  = 0;


static void setSignalsHandler();

static void handlerFIFO(int sig);

static int getRDofFIFOAccess();

static int getWDofClientAccFIFO(char *clientAccFifo);

static int getRDofClientFIFO(char *clientFifo);

static void fixFifoEof();

static void createServerFIFOAccess();

int main(int argc, const char *argv[])
{
    int serverAccRFd = -1;              /* read client accessing request    */
    int clientAccWFd    = -1;           /* write response to client         */
    int serverRFd    = -1;              /* read client data transfering     */

    struct Req req;        /* request to server from client     */
    struct AccReq accReq;
    struct Accresp accResp = {1};

    char clientFifo[CLIENT_FIFO_NAME_LEN];
    char clientAccFifo[CLIENT_FIFO_ACCESS_NAME_LEN];


/* Set signals (INT + TERM) handlers                            */

    setSignalsHandler();

/* Creating FIFO (only 1 for server to read from all clients)   */

    //createServerFIFO();

/* Creating Access FIFO (only 1 for server to read from all clients)   */

    createServerFIFOAccess();

/* Get FD to read from serverAccessFIFO                               */
 
    serverAccRFd = getRDofFIFOAccess();


/* fix that fifo may meet EOF if there are no clients           */

    fixFifoEof();

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        fprintf(stderr, "Can't set ignore to signal SISPIPE.\n");
        exit(SIGPIPE_IGN_ERROR);
    }

/* getting requests from clients                           */

    DEBPRINT("TEST: Before while(1)\n")

    while (TRUE)
    {
        /* trying to READ REQUEST from client until we got it                                           */

        if ( read(serverAccRFd, &accReq, sizeof(struct AccReq)) != sizeof(struct AccReq))
            continue;

        /* got client's request to access*/

        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) accReq.pid);
        snprintf(clientAccFifo, CLIENT_FIFO_ACCESS_NAME_LEN, CLIENT_FIFO_ACCESS_TEMPLATE, (long) accReq.pid); 

        clientAccWFd = getWDofClientAccFIFO(clientAccFifo);

        errno = 0;

        if ( write(clientAccWFd, &accResp, sizeof(struct Accresp)) != sizeof(struct Accresp))
        {
            if (errno == EPIPE)
            {
                fprintf(stderr, "Client Access fifo died or close read FD\n");
                exit(EXIT_FAILURE);
            }            

            perror("answering to client [write clietAccessFIFO]");
            continue;
        }

        int lastByteRead;
        errno = 0;

        serverRFd = getRDofClientFIFO(clientFifo); // will block unlit client will not open write-end of SERVER_FIFO

        while ( (lastByteRead = read(serverRFd, &req, BUF_SIZE)) > 0)
        {
            DEBPRINT("[%d]\n", lastByteRead);
            fprintf(stderr, "%.*s", lastByteRead, req.buffer);
        }

        if (lastByteRead != 0)
        {
            perror("reader\n");
            exit(EXIT_FAILURE);
        }

        // unlink(clientAccFifo);
        // !/ unlink(clientFifo);

        DEBPRINT("CLIENT SERVED\n");

        /* TODO: IF SERVER FIFO (ACC FIFO) were created by client that server can create his own */

    }

/* close all FD */

    DEBPRINT("TEST: HERETEST: HERETEST: HERE\n");

    ERRCHECK_CLOSE(serverAccRFd)
    ERRCHECK_CLOSE(clientAccWFd)
    ERRCHECK_CLOSE(serverRFd)


    exit(EXIT_SUCCESS);
}

static void handlerFIFO(int sig)
{
    if (NEED_UNLINK_SAFIFO)
    {
        unlink(SERVER_FIFO_ACCESS);
        DEBPRINT("SERVER ACCESS FIFO UNLINKED\n")
    }
    else
        DEBPRINT("NEED_UNLINK_SAFIFO == false\n")

    unlink(SERVER_FIFO_ACCESS);

    DEBPRINT("FIFO handler got SIGNAL: %s(%d)\n", strsignal(sig), sig);

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

static void createServerFIFOAccess()
{
    umask(0);           
    errno = 0;

    int mkfifoStatus = mkfifo(SERVER_FIFO_ACCESS, S_IRUSR | S_IWUSR | S_IWGRP);
    
    if (mkfifoStatus == -1)
    {
        if(errno != EEXIST)
        {
            perror("ERROR: mk(client)fifo access. ERROR IS NOT EEXIST\n");
            exit(MKFIFO_NO_EEXIT);
        }
    }
    else
    {
        NEED_UNLINK_SAFIFO = 1;
        DEBPRINT("NEED UNLINK A = true\n");
    }  
}

static int getRDofFIFOAccess()
{
    int serverRAccFd = open(SERVER_FIFO_ACCESS, O_RDONLY | O_NONBLOCK);
    if (serverRAccFd == -1)
    {
        fprintf(stderr, "ERROR open %s", SERVER_FIFO_ACCESS);
        exit(ERROR_OPEN_TO_READ_CLIENT);
    }

    return serverRAccFd;
}

static int getWDofClientAccFIFO(char *clientAccFifo)
{
    int clientAccWFd = open(clientAccFifo, O_WRONLY);
    if (clientAccWFd == -1)
    {
        fprintf(stderr, "clientAccFifo = %s\n", clientAccFifo);
        perror("clientAccFifo");
        exit(EXIT_FAILURE);
    }

    return clientAccWFd;
}

static int getRDofClientFIFO(char *clientFifo)
{ 
    /* blocked while write-end close. To fix endless blocking we must open write-end */

    int serverRFd = open(clientFifo, O_RDONLY); //| O_NONBLOCK);  
    if (serverRFd == -1)
    {
        fprintf(stderr, "clientAccFifo = %s\n", clientFifo);
        perror("clientFifo");
        exit(ERROR_OPEN_TO_READ_CLIENT);
    }

    return serverRFd;
}
