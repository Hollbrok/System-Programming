#include "libs.h"
#include "commun.h"

static char clientFifo[CLIENT_FIFO_NAME_LEN];

static int NEED_UNLINK_SAFIFO  = 0;

static void handlerFIFO(int sig);

static void setSignalsHandler();

static void checkargv(int argc, const char *argv[]);

static void createServerFIFOAccess();

static void createClientFIFO();

static void removeFifo(void);

static int getWDofClientFIFO();

int main(int argc, const char *argv[])
{
    checkargv(argc, argv);

    struct Req req          = {};
    struct AccReq accReq    = {getpid()};
 
    int serverAccWFd    = -1;      /* write to server access fifo */
    int fileRFd         = -1;      /* read from file */
    int clientWFd       = -1;      /* transfer data from client(file) to server */
     
    int lastByteRead    = -1; 
    int lastByteWrite   = -1;

/* Set signals (INT + TERM) handlers                    */

    setSignalsHandler();

/* creates FIFO's for read access and write data from file to server */ 

    createClientFIFO();   

/*  */

    DEBPRINT("opening serverFIFO  access on write\n")

    serverAccWFd = open(SERVER_FIFO_ACCESS, O_WRONLY);  /* in block till server doesn't open on read or error if SERVER FIFO doesn't exists*/      
    
    DEBPRINT("after open SERVER_FIFO_ACCESS on write\n");
   
    if (serverAccWFd == -1)
    {   
        DEBPRINT("server fifo access FD == -1\n");

        if (errno != ENOENT)
        {
            perror("ERROR in open on write server access FIFO\n");
            exit(EXIT_FAILURE); // add special error-name
        }

        /* here errno == ENOENT. It means no such file or dir. So server doesn't exist yet and we must create serverFIFOACCESS it our's own*/

        createServerFIFOAccess();
        if ( (serverAccWFd = open(SERVER_FIFO_ACCESS, O_WRONLY)) == -1) /* now can only be in block till server will not open read-end */
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

/* writing request on access to transfer data */

/*

    sleep(5);

    int testWd = open(clientFifo, O_WRONLY | O_NONBLOCK);
    if ( testWd == -1 )
    {
        DEBPRINT("ALARM!!!!!. clientFifo = %s\n", clientFifo)
        perror("");
        exit(EXIT_FAILURE);
    }

*/

    if ( (lastByteWrite = write(serverAccWFd, &accReq, sizeof(struct AccReq))) != sizeof(struct AccReq) )
    {
        perror("Error in write to SERVER_FIFO_ACCESS");
        exit(EXIT_FAILURE); 
    }
    else
        DEBPRINT("successful write request to server\n")


/* ignore SIGPIPE so we will catch EPIPE if read-end of clientFIFO are closed while writing to clientFIFO instead of SIGPIPE that will end our process */

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        fprintf(stderr, "Can't set ignore to signal SISPIPE.\n");
        exit(SIGPIPE_IGN_ERROR);
    }

/* open CF on write*/

    clientWFd = getWDofClientFIFO(); 

/*

    ERRCHECK_CLOSE(testWd)

*/

    DEBPRINT("After open clientFifo on write\n");

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
    ERRCHECK_CLOSE(serverAccWFd)
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

static void createClientFIFO()
{
    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) getpid());

    DEBPRINT("before mkfifo\n")

    if ( mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
    {
        if(errno != EEXIST)
        {
            fprintf(stderr, "ERROR: mk(client)fifo. ERROR IS NOT EEXIST\n");
            exit(MKFIFO_NO_EEXIT);
        }
        /* else all good */
    }   
    else
        DEBPRINT("successfull mkfile clientFifo = %s\n", clientFifo)
}

static void removeFifo(void)
{
    DEBPRINT("TEST: IN REMOVE FIFO\n")

    unlink(clientFifo);
    DEBPRINT("clientFifo unlinked\n")

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

static int getWDofClientFIFO()
{
    errno = 0;
    int attemps = 3;

    int clientWFd = open(clientFifo, O_WRONLY | O_NONBLOCK); // may be deadlock if server died 

    DEBPRINT("After open clientFifo on write+O_NONBLOCK\n");

    if (clientWFd == -1)
    {
        if (errno == ENOENT)
        {
            perror("client fifo doesn't exists\n");
            exit(EXIT_FAILURE);
        }
        while (errno  == ENXIO && attemps)
        {
            sleep(1);
            --attemps;
            errno = 0;
            clientWFd = open(clientFifo, O_WRONLY | O_NONBLOCK);    
        }

        if (attemps != 0)
        {
            DEBPRINT("successful opened on write client FIFO\n")
        }
        else
        {
            DEBPRINT("WAIT TIME ended\n")
            return -1;
        }
    }

    errno = 0;

    fcntl(clientWFd, F_SETFL, O_RDONLY);//(fcntl(clientWFd, F_GETFL, NULL) & ~O_NONBLOCK));

    if (errno == -1)
    {
        DEBPRINT("can't disable O_NONBLOCK\n")
        exit(EXIT_FAILURE);
    }

    DEBPRINT("O_NONBLOCK disabled\n")
    
    return clientWFd;

}


