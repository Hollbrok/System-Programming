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
    int serverAccRFd    = -1;              /* read client accessing request     */
    int clientAccWFd    = -1;              /* write response to client          */
    int serverRFd       = -1;              /* read client data transfering      */

    struct Req req;        /* request to server from client     */
    struct AccReq accReq;
    struct Accresp accResp = {1};

    char clientFifo[CLIENT_FIFO_NAME_LEN];
    char clientAccFifo[CLIENT_FIFO_ACCESS_NAME_LEN];

/* Set signals (INT + TERM) handlers                            */

    setSignalsHandler();

/* Creating Access FIFO (only 1 for server to read from all clients)   */

    createServerFIFOAccess();

/* Get FD to read from serverAccessFIFO                               */
 
    serverAccRFd = getRDofFIFOAccess();

/* fix that fifo may meet EOF if there are no clients (to wait for new clients in cycle) */

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
        
        DEBPRINT("[test]\n")

        int testRead;

        if ( (testRead = (read(serverAccRFd, &accReq, sizeof(struct AccReq)) )) != sizeof(struct AccReq))
        {
            DEBPRINT("testRead = %d\n", testRead);
            continue;
        }

        DEBPRINT("testRead = %d\n", testRead);


        /* got client's request to access*/

        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) accReq.pid);
        snprintf(clientAccFifo, CLIENT_FIFO_ACCESS_NAME_LEN, CLIENT_FIFO_ACCESS_TEMPLATE, (long) accReq.pid); 

        //DEB_SLEEP(5, "test1: DO INT CLIENT\n")

        if ( (clientAccWFd = getWDofClientAccFIFO(clientAccFifo)) == -1)
        {
            /* read-end of client acc fifo closed --> let's serve another one */
            DEBPRINT("can't get any actions from clients. Go to server next\n")
            //fprintf(stderr, "test1: success\n");
            continue;
        }

        errno = 0;

        DEBPRINT("writing on CLIENT_ACC_FIFO\n")

        int test_write = 0;

        if ( (test_write = write(clientAccWFd, &accResp, sizeof(struct Accresp))) != sizeof(struct Accresp))
        {
            if (errno == EPIPE)
            {
                fprintf(stderr, "Client Access fifo died or close read FD\n");
                exit(EXIT_FAILURE);
            }            

            perror("answering to client [write clietAccessFIFO]");
            continue;
        }

        DEBPRINT("after writing on CLIENT_ACC_FIFO. test_write = %d\n", test_write)       

        int lastByteRead;
        errno = 0;

        serverRFd = getRDofClientFIFO(clientFifo); 

        if (serverRFd == -1)
        {
            DEBPRINT("client can't open write-end of fifo\n")
            continue;
        }

        while ( (lastByteRead = read(serverRFd, &req, BUF_SIZE)) > 0 )
        {
            DEBPRINT("[%d]\n", lastByteRead);
            fprintf(stderr, "%.*s", lastByteRead, req.buffer);
        }

        if (lastByteRead != 0)
        { 
            perror("reader\n");
            exit(EXIT_FAILURE);
        }

        DEBPRINT("CLIENT SERVED, LBR = %d\n", lastByteRead);
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
    /* this is FD to fix problem with FIFO: it needs to open FIFO write end to not meet EOF. */

    int fixAccFd  = -1;    

    fixAccFd = open(SERVER_FIFO_ACCESS, O_WRONLY);

    /* got here only after client client open read-end*/

    if (fixAccFd  == -1)
    {
        fprintf(stderr, "ERROR: open fixFD. %s\n", SERVER_FIFO_ACCESS);
        exit(OPEN_FIX_FD);
    }
}

static void createServerFIFOAccess()
{
    errno = 0;

    int mkfifoStatus = mkfifo(SERVER_FIFO_ACCESS, S_IRUSR | S_IWUSR | S_IWGRP);
    
    if (mkfifoStatus == -1)
    {
        NEED_UNLINK_SAFIFO = 1; /* test */
        if(errno != EEXIST)
        {
            perror("ERROR: mk(client)fifo access. ERROR IS NOT EEXIST\n");
            exit(MKFIFO_NO_EEXIT);
        }
    }
    else
    {
        NEED_UNLINK_SAFIFO = 1;
        DEBPRINT("NEED UNLINK SAFIFO = true\n");
    }  
}

static int getRDofFIFOAccess()
{
    /* will in block till here no client */

    int serverRAccFd = open(SERVER_FIFO_ACCESS, O_RDONLY);
    if (serverRAccFd == -1)
    {
        fprintf(stderr, "ERROR open %s", SERVER_FIFO_ACCESS);
        exit(ERROR_OPEN_TO_READ_CLIENT);
    }

    DEBPRINT("GOT READ END OF SERVER FIFO ACCESS\n")

    return serverRAccFd;
}

static int getWDofClientAccFIFO(char *clientAccFifo)
{
    int clientAccWFd = open(clientAccFifo, O_WRONLY | O_NONBLOCK);

    DEBPRINT("after open CAF on W|NB\n");

    if (clientAccWFd == -1)
    {   
        struct timeval waitTime = {};
        waitTime.tv_sec = 3;

        fd_set writeFDs = {};
        FD_ZERO(&writeFDs);
        FD_SET(getpid(), &writeFDs);// FD_SET(clientAccWFd, &writeFDs);

        DEBPRINT("[getting R] do select\n");
        if ( (select(getpid() + 1, NULL, &writeFDs, NULL, &waitTime)) > 0)// ( (select(clientAccWFd + 1, NULL, &writeFDs, NULL, &waitTime)) > 0)
        {
            DEBPRINT("successful select\n")
            return open(clientAccFifo, O_WRONLY);//return clientAccWFd; 
        }
        else 
        {
            perror("TEST");
            DEBPRINT("select failed\n");
            return -1;
        }
    }
    else
    {
        DEBPRINT("SUCCESSFUL open CAF (read-end opened by client)\n");
        return clientAccWFd;
    }
}

static int getRDofClientFIFO(char *clientFifo)
{ 
    /* blocked while write-end close. To fix endless blocking we must open write-end */

    DEBPRINT("before opening clientFIFO\n")
    
    int serverRFd = open(clientFifo, O_RDONLY | O_NONBLOCK);  // EAGAIN if server'll read from empty fifo with write end open

    //DEB_SLEEP(2, "after opening clientFIFO\n")

    if (serverRFd == -1)
    {   
        perror("should never get here!!!!!!!!!!!!!!!!!\n");
        
        exit(EXIT_FAILURE);

    }
    else
    {
        DEBPRINT("SUCCESSFUL open CF (read-end opened by client)\n");

        /* off O_NONBLOCKING  (!!!!)*/
        
        DEB_SLEEP(5, "OFF NON_BLOCKING [TEST: try to close client]\n");
        
        errno = 0;

        fcntl(serverRFd, F_SETFL, O_RDONLY);

        if (errno == 0)
            DEBPRINT("SUCCESS OFFED\n")
        else 
            DEBPRINT("NOT SUCCESS OFFED\n")

        return serverRFd;
    }
    
   /* {
        fprintf(stderr, "clientAccFifo = %s\n", clientFifo);
        perror("clientFifo");
        exit(ERROR_OPEN_TO_READ_CLIENT);
    } 

    return serverRFd; */
}
