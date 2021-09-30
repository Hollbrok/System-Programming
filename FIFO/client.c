#include "libs.h"
#include "commun.h"

static char clientFifo[CLIENT_FIFO_NAME_LEN];



/* on succ-exit must delete client FIFO */

static void removeFifo(void);

static void checkargv(int argc, const char *argv[]);

static void createServerFIFOAccess();

static void createServerFIFO();

static void createClientFIFO();

int main(int argc, const char *argv[])
{
    checkargv(argc, argv);

    struct Req req;
    struct AccReq accReq = {getpid()};
    struct Accresp accResp = {1};

    int serverAccWFd = -1;      // write to server access fifo
    int clientRFd    = -1;      // reding access from server
    
    int fileRFd      = -1;      // read from file
    int serverWFd    = -1;      // transfer data from file to server
    

    int lastByteRead; 
    int lastByteWrite;

/* we get the permissions on file creating that we want */

    umask(0); 

    createClientFIFO();

/* process is blocked until server opens on read */


    serverAccWFd = open(SERVER_FIFO_ACCESS, O_WRONLY);          
    
    DEBPRINT("After open SERVER_FIFO_ACCESS on write\n");
   
    if (serverAccWFd == -1)
    {   
        if(errno != ENOENT)
        {
            perror("ERROR in open on write server access FIFO\n");
            exit(EXIT_FAILURE); // add special error-name
        }

        /* errno == ENOENT. It means no such file or dir. So server doesn't exist yet and we must create serverFIFOACCESS it our own*/

        createServerFIFOAccess();
        serverAccWFd = open(SERVER_FIFO_ACCESS, O_WRONLY);

        /*if ( (serverWFd = open(SERVER_FIFO, O_WRONLY)) == -1 && errno != ENOENT)
        {
            perror("ERROR in open on write server FIFO\n");
            exit(EXIT_FAILURE); // add special error-name
        } */    
    }

    if ( (fileRFd = open(argv[1], O_RDONLY)) == -1)
    {
        perror("Can't open file on read\n");
        exit(EXIT_FAILURE);
    } 


/* Getting access to write in real FIFO*/

    if ( (lastByteWrite = write(serverAccWFd, &accReq, sizeof(struct AccReq))) != sizeof(struct AccReq) )
    {
        perror("Error in write to SERVER_FIFO_ACCESS");
        exit(EXIT_FAILURE); 
    }

    lastByteWrite = 0;

    clientRFd = open(clientFifo, O_RDONLY); // blocking while server dont open another end of FIFO ==== we get access on write to transfer data to server
    
    DEBPRINT("After open clienFIFO on read\n");

    if (clientRFd < 0)
    {
        perror("client fifo on READ");
        exit(EXIT_FAILURE);
    }

/* get access from server */

    if ( (lastByteRead = read(clientRFd, &accResp, sizeof(struct Accresp))) != sizeof(struct Accresp) )
    {
        perror("Error in read from client FIFO");
        exit(EXIT_FAILURE); 
    }

/* ignore SIGPIPE so we will catch EPIPE if read-end of SF will close instead of SIGPIPE that will end our process*/

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        fprintf(stderr, "Can't set ignore to signal SISPIPE.\n");
        exit(SIGPIPE_IGN_ERROR);
    }

/* open SF on write*/

    serverWFd = open(SERVER_FIFO, O_WRONLY); // a lot of client can open FIFO
    if (serverWFd == -1)
    {
        if(errno != ENOENT)
        {
            perror("ERROR in open on write server FIFO\n");
            exit(EXIT_FAILURE); // add special error-name
        }

        /* errno == ENOENT. It means no such file or dir. So server doesn't exist yet and we must create serverFIFO*/

        createServerFIFO();
        
        /*if ( (serverWFd = open(SERVER_FIFO, O_WRONLY)) == -1 && errno != ENOENT)
        {
            perror("ERROR in open on write server FIFO\n");
            exit(EXIT_FAILURE); // add special error-name
        } */    
    }

    lastByteRead = 0;
    errno = 0;
    
    /* read from file, write to server FIFO */

    while ( (lastByteRead = read(fileRFd, req.buffer, BUF_SIZE)) > 0 )
    {
        if ( write(serverWFd, req.buffer, lastByteRead) != lastByteRead )
        {
            if(errno == EPIPE)
            {
                fprintf(stderr, "server died or closed read-end of FIFO\n");
                exit(EXIT_FAILURE);
            }

            fprintf(stderr, "Can't write to server FIFO\n");
            exit(EXIT_FAILURE); // add special error-name
        }
    }



    if (lastByteRead != 0)
    {
        perror("reader\n");
        exit(EXIT_FAILURE);
    }

/* closes all FD's */

    ERRCHECK_CLOSE(serverWFd)
    ERRCHECK_CLOSE(fileRFd)
    ERRCHECK_CLOSE(serverAccWFd)
    ERRCHECK_CLOSE(clientRFd)
    
    DEBPRINT("SECCUSSFUL\n");

    exit(EXIT_SUCCESS);
}

static void removeFifo(void)
{
    fprintf(stderr, "TEST: IN REMOVE FIFO\n");
    unlink(clientFifo);
    fprintf(stderr, "clientFifo unlinked\n");
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
}

static void createServerFIFO()
{
    umask(0);           
    errno = 0;

    int mkfifoStatus = mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP);

    if (mkfifoStatus == -1)
    {
        if(errno != EEXIST)
        {
            perror("ERROR: mk(client)fifo. ERROR IS NOT EEXIST\n");
            exit(MKFIFO_NO_EEXIT);
        }
    }  
}

static void createClientFIFO()
{
    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) getpid());

    if ( mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1
         && errno != EEXIST)
    {
        fprintf(stderr, "ERROR: mk(client)fifo. ERROR IS NOT EEXIST\n");
        exit(MKFIFO_NO_EEXIT);
    }   

    if (atexit(removeFifo) != 0)
    {
        fprintf(stderr, "Can't set exit function\n");
        exit(EXIT_FAILURE);
    }  
}



