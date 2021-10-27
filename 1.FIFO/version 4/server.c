#include "libs.h"
#include "commun.h"
 
static int NEED_UNLINK_SAFIFO  = 0;
static int NEED_UNLINK_SFIFO   = 0;


static char SERVER_FIFO[SERVER_FIFO_NAME_LEN];


static void handlerFIFO(int sig);

static void setSignalsHandler();

static int fixFifoEof();

static void createServerFIFOAccess();

static void createServerFifo(char* serverFifo);

static int getWDofFIFOAccess();

static int getRDofServerFIFO(char *clientFifo); 

int main(int argc, const char *argv[])
{
    int serverAccWFd    = -1;              /* write PID to server FIFO      */
    int serverRFd       = -1;              /* read client data transfering  */

    int fixFD           = -1;

    struct Req req;                        /* request to server from client     */
    struct AccResp accResp = {getpid()};   /* access request to server from client [PID] to know fifo */

    char serverFifo[SERVER_FIFO_NAME_LEN];

    snprintf(serverFifo, SERVER_FIFO_NAME_LEN, SERVER_FIFO_TEMPLATE, (long) getpid());

/* Set signals (INT + TERM) handlers [rather for debugging]             */

    setSignalsHandler();

/* Creating Access FIFO (only 1 for server to read from all clients)    */

    createServerFIFOAccess();

/*  create serverFIFO*/

    createServerFifo(serverFifo);

/* serving clients in loop */

    DEBPRINT("Before while(1)\n")

    int isSuccess = 0;

///////////////////////////////////////////////////////////////

    do /* only 1 iteration */
    {
        DEBPRINT("At the start of while\n");

    /* Get FD to write to serverAccessFIFO  */
        serverAccWFd = getWDofFIFOAccess();

    /* Get read-end of serverFIFO           */
        serverRFd    = getRDofServerFIFO(serverFifo); 


        /* WRITE REQUEST [PID] to FIFO it equals that we are ready to server client*/
        
        DEBPRINT("writing pid to server ACC FIFO\n")

        if ( write(serverAccWFd, &accResp, sizeof(struct AccResp)) != sizeof(struct AccResp)) /* get SIGPIPE + EPIPE of read end closed */
        {
            DEBPRINT("read from serverAcc FIFO != sizeof(struct AccResp). LINE = %d\n", __LINE__);
            break;//continue;
        }

        DEBPRINT("after writing PID \n")


        int lastByteRead = -1;
        errno = 0;
 
    //!!!!  Client has 1 sec to open write-end of serverFIFO
    //!!!!
        sleep(1);
    //!!!!
    //!!!!

        while ( ((lastByteRead = read(serverRFd, &req, BUF_SIZE)) > 0 ) )
        {
            DEBPRINT("[%d]\n", lastByteRead); 
        
            if (write(STDERR_FILENO, req.buffer, lastByteRead) == -1)
            {
                perror("write to stdout");
                exit(EXIT_FAILURE);
            }

        }

        if (lastByteRead != 0)
        { 
            perror("read from client FIFO failed\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            isSuccess = 1;
            DEBPRINT("Got EOF (successfull file transfer)\n");
        }

        ERRCHECK_CLOSE(serverAccWFd);
        ERRCHECK_CLOSE(serverRFd);

        DEBPRINT("CLIENT SERVED. LBR = %d\n", lastByteRead);

    } while(0);

    fprintf(stdout, "STATUS of transfer: %s\n", isSuccess ? "success" : "failed" ); 

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

    if (NEED_UNLINK_SFIFO)
    {
        unlink(SERVER_FIFO);
    }
    else
    {
        DEBPRINT("NEED_UNLINK_SFIFO == false\n")
    }

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

static int fixFifoEof()
{

    int fixAccFd  = -1;    

    fixAccFd = open(SERVER_FIFO_ACCESS, O_RDONLY);

/* read-end already opened */

    if (fixAccFd  == -1)
    {
        fprintf(stderr, "ERROR: open fixFD. %s\n", SERVER_FIFO_ACCESS);
        exit(OPEN_FIX_FD);
    }

    return fixAccFd;
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

static void createServerFifo(char* serverFifo)
{
    //snprintf(serverFifo, SERVER_FIFO_NAME_LEN, SERVER_FIFO_TEMPLATE, (long) pid);

    DEBPRINT("before mkfifo\n")

    if ( mkfifo(serverFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
    {
        if(errno != EEXIST)
        {
            fprintf(stderr, "ERROR: mk(client)fifo. ERROR IS NOT EEXIST\n");
            exit(MKFIFO_NO_EEXIT);
        }
        /* else all good */
    }   
    else
    {
        strncpy(SERVER_FIFO, serverFifo, strlen(serverFifo));
        
        NEED_UNLINK_SFIFO = 1;
        DEBPRINT("successfull mkfile serverFifo = %s\n", serverFifo)
    }
}

static int getWDofFIFOAccess()
{
    /* will in block till here no client */

    int serverRAccFd = open(SERVER_FIFO_ACCESS, O_WRONLY);
    if (serverRAccFd == -1)
    {
        fprintf(stderr, "ERROR open %s", SERVER_FIFO_ACCESS);
        exit(ERROR_OPEN_TO_READ_CLIENT);
    }

    DEBPRINT("GOT READ END OF SERVER FIFO ACCESS\n")

    return serverRAccFd;
}

static int getRDofServerFIFO(char *serverFifo)
{ 
    int savedErrno = errno;
    errno = 0;

    DEBPRINT("before  opening serverFIFO on read\n")
    
    int serverRFd = open(serverFifo, O_RDONLY | O_NONBLOCK); 


    if (serverRFd == -1) /* */
    {       
        perror("Can't open serverFIFO on read+nonblock\n");
        exit(EXIT_FAILURE);
    }
    else /* client fifo exists and write-end opened */
    {
        DEBPRINT("open client FIFO on read (read-end opened by client)\n");

        /* disable O_NONBLOCK to get block if FIFO is empty and write end open (if close will got 0(EOF) from read) */
        
        fcntl(serverRFd, F_SETFL, O_RDONLY); //(fcntl(serverRFd, F_GETFL, NULL) & ~O_NONBLOCK) );
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
