#include "libs.h"
#include "commun.h"
  
static char serverFifo[SERVER_FIFO_NAME_LEN];

static int NEED_UNLINK_SAFIFO  = 0;


static void handlerFIFO(int sig);

static void setSignalsHandler();
 
static void checkargv(int argc, const char *argv[]);

static void createServerFIFOAccess();

static void removeFifo(void);

static int getWDofServerFIFO(pid_t pid);

int main(int argc, const char *argv[])
{
    checkargv(argc, argv);

    struct Req req          = {};
    struct AccResp accResp  = {};
 
    int serverAccRFd    = -1;      /* read from server access fifo */

    int fileRFd         = -1;      /* read from file */
    int clientWFd       = -1;      /* transfer data from client(file) to server */
     
    int lastByteRead    = -1; 
    int lastByteWrite   = -1;

/* Set signals (INT + TERM) handlers [rather for debugging] */

    setSignalsHandler();

/*  getting response from server to known pid */

    DEBPRINT("opening serverFIFO  access on read\n")

    serverAccRFd = open(SERVER_FIFO_ACCESS, O_RDONLY);  /* in block till server doesn't open on write or error if SERVER FIFO doesn't exists*/      
    
    DEBPRINT("after open SERVER_FIFO_ACCESS on read\n");
   
    if (serverAccRFd == -1)
    {   
        DEBPRINT("server fifo access FD == -1\n");

        if (errno != ENOENT)
        {
            perror("ERROR in open on read server access FIFO\n");
            exit(EXIT_FAILURE);
        }

        /* here errno == ENOENT. It means no such file or dir. So server doesn't exist yet and we must create serverFIFOACCESS it our's own*/

        createServerFIFOAccess();

        if ( (serverAccRFd = open(SERVER_FIFO_ACCESS, O_RDONLY)) == -1) /* now can only be in block till server will not open write-end */
        {
            perror("erron in open serverFIFO access on write\n");
            exit(EXIT_FAILURE);
        }

        DEBPRINT("\"REAL\" opened SERVER_FIFO_ACCESS on write\n");    
    } 

/* open file with data */

    if ( (fileRFd = open(argv[1], O_RDONLY)) == -1)
    {
        perror("Can't open file on read\n");
        exit(EXIT_FAILURE);
    } 

/* ignore SIGPIPE so we will catch EPIPE if read-end of serverFifo are closed while writing to serverFifo instead of SIGPIPE that will end our process */

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        fprintf(stderr, "Can't set ignore to signal SISPIPE.\n");
        exit(SIGPIPE_IGN_ERROR);
    }
 
/* reading request on access to transfer data */

    errno = 0;

    if ( (lastByteWrite = read(serverAccRFd, &accResp, sizeof(struct AccResp))) != sizeof(struct AccResp) )
    {
        perror("Error in read from SERVER_FIFO_ACCESS");
        DEBPRINT("LBR = %d\n", lastByteWrite);
        exit(EXIT_FAILURE); 
    }
    else
        DEBPRINT("successful read request to server\n")

/* open CF on write*/

    clientWFd = getWDofServerFIFO(accResp.pid); 

    DEBPRINT("After open serverFifo on write\n");

    if (clientWFd == -1)
    {
        DEBPRINT("server probably died\n");
        exit(EXIT_FAILURE);    
    }

    lastByteRead = 0;
    lastByteWrite = 0;
    errno = 0;
    
    /* read from file, write to server FIFO                                     */
    /* if read-end are closed we got EPIPE error (+SIGPIPE, but we ignore them) */

    while ( (lastByteRead = read(fileRFd, req.buffer, BUF_SIZE)) > 0 )
    {
        if ( write(clientWFd, req.buffer, lastByteRead) != lastByteRead )
        {
            if (errno == EPIPE) /* so server died (read-end closed) */
            {
                fprintf(stderr, "server died or closed read-end of FIFO\n");
                exit(EXIT_FAILURE);
            }

            fprintf(stderr, "Can't write to server FIFO\n");
            exit(EXIT_FAILURE);
        }
    }

    if (lastByteRead != 0)
    {
        perror("reader\n");
        exit(EXIT_FAILURE);
    }

/* closes all FD's */

    ERRCHECK_CLOSE(clientWFd)
    ERRCHECK_CLOSE(serverAccRFd)
    ERRCHECK_CLOSE(fileRFd)

    DEBPRINT("SECCUSSFUL\n");

    exit(EXIT_SUCCESS);
}

static void checkargv(int argc, const char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Program needs 2 arguments\n");
        exit(NOT_ENOUGH_ARGUMENTS);
    }
    
    if (argc > 2)
    {
        fprintf(stderr, "Too many arguments (need 1 number)\n");
        exit(TO_MUCH_ARGUMENTS);
    }

    if (*argv[1] == '\0')
    {
        fprintf(stderr, "zero string argv2\n");
        exit(ZEROSTRING_ARGV);
    }
}

static void createServerFIFOAccess()
{           
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
    else /* belonging serverACCESSFIFO to client not to server */
    {
        DEBPRINT("Server Access fifo was created by client\n");
        
        NEED_UNLINK_SAFIFO = 0; //  (=0 TEST)
    }
}

static void removeFifo(void)
{
    //DEBPRINT("TEST: IN REMOVE FIFO\n")

    //unlink(serverFifo);
    //DEBPRINT("serverFifo unlinked\n")

    if (NEED_UNLINK_SAFIFO)
    {
        /* need to check if server created and only after unlink*/
        unlink(SERVER_FIFO_ACCESS);
        DEBPRINT("SERVER ACCESS FIFO unlinked\n")
    }
}

static void handlerFIFO(int sig)
{
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
    
    if (atexit(removeFifo) != 0)
    {
        fprintf(stderr, "Can't set exit function\n");
        exit(EXIT_FAILURE);
    }  
    else
        DEBPRINT("Successful set removeFIFO\n")
}

static int getWDofServerFIFO(pid_t pid)
{
    errno = 0;

    snprintf(serverFifo, SERVER_FIFO_NAME_LEN, SERVER_FIFO_TEMPLATE, (long) pid);

    int serverWFd = open(serverFifo, O_WRONLY | O_NONBLOCK); // read-end of serverFIFO   ALWAYS opened on this step!! 

    DEBPRINT("After open serverFifo on write+O_NONBLOCK\n");

    if (serverWFd == -1)
    {
        if (errno == ENOENT)
        {
            perror("client fifo doesn't exists\n");
            exit(EXIT_FAILURE);
        }
        if (errno == ENXIO) /* server died*/
        {
            DEBPRINT("server died\n")
            exit(EXIT_FAILURE);
        }

        DEBPRINT("UNDEFINIED ERROR. exit..\n")
        exit(EXIT_FAILURE);
    }

    errno = 0;

    fcntl(serverWFd, F_SETFL, (fcntl(serverWFd, F_GETFL, NULL) & ~O_NONBLOCK));

    if (errno == -1)
    {
        DEBPRINT("can't disable O_NONBLOCK\n")
        exit(EXIT_FAILURE);
    }

    DEBPRINT("O_NONBLOCK disabled\n")
    
    return serverWFd;

}


