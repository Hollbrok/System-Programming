#include "libs.h"
#include "commun.h"

static int NEED_UNLINK_SAFIFO  = 0;
static int NEED_UNLINK_SFIFO   = 0;

static void handlerFIFO(int sig);

static void setSignalsHandler();

static void fixFifoEof();

static void createServerFIFOAccess();

static int getRDofFIFOAccess();

static int getRDofClientFIFO(char *clientFifo); 

int main(int argc, const char *argv[])
{
    int serverAccRFd    = -1;              /* read client accessing request     */
    int serverRFd       = -1;              /* read client data transfering      */

    struct Req req;                        /* request to server from client     */
    struct AccReq accReq;                  /* access request to server from client [PID] to know fifo */

    char clientFifo[CLIENT_FIFO_NAME_LEN];

/* Set signals (INT + TERM) handlers                                    */

    setSignalsHandler();

/* Creating Access FIFO (only 1 for server to read from all clients)    */

    createServerFIFOAccess();
    
/* Get FD to read from serverAccessFIFO                                 */
 
    serverAccRFd = getRDofFIFOAccess();

/* fix that fifo may meet EOF if there are no clients (to wait for new clients in cycle) */

    fixFifoEof();

/* serving clients in loop */

    DEBPRINT("Before while(1)\n")

    while (TRUE)
    {
        /* trying to READ REQUEST from client until we got it           */
        
        DEBPRINT("at the start of while(true)\n")

        if ( read(serverAccRFd, &accReq, sizeof(struct AccReq)) != sizeof(struct AccReq))
        {
            DEBPRINT("read from serverAcc FIFO != sizeof(struct AccReq). LINE = %d\n", __LINE__);
            continue;
        }

        DEBPRINT("after got client\n")

        /* got client's request so lets open on write his FIFO */

        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) accReq.pid);

        int lastByteRead = -1;
        errno = 0;

        serverRFd = getRDofClientFIFO(clientFifo); 

        if (serverRFd == -1)
        {
            DEBPRINT("SHOULD NEVER BE HERE!\n"
                     "client can't open write-end of fifo\n")
            continue;
        }

        while ( ((lastByteRead = read(serverRFd, &req, BUF_SIZE)) > 0 ) ) // || (errno == EAGAIN) )
        {
            DEBPRINT("[%d]\n", lastByteRead);
            fprintf(stderr, "%.*s", lastByteRead, req.buffer);
        }

        if (lastByteRead != 0)
        { 
            perror("read from client FIFO failed\n");
            exit(EXIT_FAILURE);
        }

        DEBPRINT("CLIENT SERVED\n");
    }


    DEBPRINT("SUCCESSFUL COMPLETED")
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

    exit(EXIT_SUCCESS);
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
/* this is FD to fix problem with FIFO: it needs to open FIFO write end to not meet EOF.  switch behavior to block*/

    int fixAccFd  = -1;    

    fixAccFd = open(SERVER_FIFO_ACCESS, O_WRONLY);

/* read-end already opened */

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

static int getRDofClientFIFO(char *clientFifo)
{ 
    int savedErrno = errno;
    errno = 0;

    DEBPRINT("before opening clientFIFO on read\n")
    
    int serverRFd = open(clientFifo, O_RDONLY | O_NONBLOCK); 

    //DEB_SLEEP(2, "after opening clientFIFO\n")

    if (serverRFd == -1) /* */
    {       
        DEBPRINT("Can't open clientFIFO on read+nonblock\n")
        exit(EXIT_FAILURE);
    }
    else /* client fifo exists and write-end opened */
    {
        DEBPRINT("open client FIFO on read (read-end opened by client)\n");

        /* disable O_NONBLOCK to get block if FIFO is empty and write end open (if close will got 0(EOF) from read) */
        
        //DEB_SLEEP(5, "OFF NON_BLOCKING [TEST: try to close client]\n");

        fcntl(serverRFd, F_SETFL, (fcntl(serverRFd, F_GETFL, NULL) & ~O_NONBLOCK) );
        if (errno == -1)
        {
            DEBPRINT("can't disable O_NONBLOCK\n")
            exit(EXIT_FAILURE);
        }

        DEBPRINT("O_NONBLOCK in read from client disabled\n");

        errno = savedErrno;
        return serverRFd;
    }
    
}
