#include "libs.h"
#include "commun.h"

static char clientFifo[CLIENT_FIFO_NAME_LEN];

/* on succ-exit must delete client FIFO */

static void removeFifo(void);

static long getNumber(const char *numString);

static void checkargv(int argc, const char *argv[]);

static void createServerFIFOAccess();

static void createServerFIFO();

int main(int argc, const char *argv[])
{
    checkargv(argc, argv);

    struct Req req;
    struct AccReq accReq = {getpid()};
    struct Accresp accResp = {1};

    int serverWFd = -1;     // write request
    int serverAccWFd = -1;  // write to server access fifo

    int fileRD = -1;        // read from file 
    int lastByteRead;       // 
    int lastByteWrite;

    //for (int i = 0; i < BUF_SIZE; i++)
     //   req.buffer[i] = 'x';
    //req.buffer[BUF_SIZE - 1] = '\0';        /* <-- Useless*/

    umask(0); /* we get the permissions we want */
    
    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) getpid());

    if (mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1
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

    serverAccWFd = open(SERVER_FIFO_ACCESS, O_WRONLY);
    if (serverAccWFd == -1)
    {   
        if(errno != ENOENT)
        {
            perror("ERROR in open on write server access FIFO\n");
            exit(EXIT_FAILURE); // add special error-name
        }

        createServerFIFOAccess();
    }

    if ( (fileRD = open(argv[1], O_RDONLY)) == -1)
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
    

    int clientRFd = open(clientFifo, O_RDONLY); // blocking while server dont open another end of FIFO
    if (clientRFd < 0)
    {
        perror("client fifo on READ");
        exit(EXIT_FAILURE);
    }

    if ( (lastByteRead = read(clientRFd, &accResp, sizeof(struct Accresp))) != sizeof(struct Accresp) )
    {
        perror("Error in read from client FIFO");
        exit(EXIT_FAILURE); 
    }

    serverWFd = open(SERVER_FIFO, O_WRONLY); // a lot of client can open FIFO
    if (serverWFd == -1)
    {
        if(errno != ENOENT)
        {
            perror("ERROR in open on write server FIFO\n");
            exit(EXIT_FAILURE); // add special error-name
        }

        createServerFIFO();
        //fprintf(stderr, "ERROR in open on write server FIFO\n");
        //exit(EXIT_FAILURE); // add special error-name
    }

    lastByteRead = 0;

    errno = 0;
    
    while ( (lastByteRead = read(fileRD, req.buffer, BUF_SIZE)) > 0 )
    {
        //req.realSize = lastByteRead;
        //req.pid = getpid();

        if ( write(serverWFd, req.buffer, lastByteRead) != lastByteRead )
        {
            fprintf(stderr, "Can't write to server FIFO\n");
            exit(EXIT_FAILURE); // add special error-name
        }
        //++req.NOaccess;
    }

    if (lastByteRead != 0)
    {
        perror("reader\n");
        exit(EXIT_FAILURE);
    }

    if (close(serverWFd) != 0)
    {
        perror("Close serverWFd");
    }

    if (close(fileRD) != 0)
    {
        perror("Close fileRD");
    }

    if (close(serverAccWFd) != 0)
    {
        perror("Close fileRD");
    }

    if (close(clientRFd) != 0)
    {
        perror("Close fileRD");
    }
    

    fprintf(stderr, "SECCUSSFUL\n");

    exit(EXIT_SUCCESS);
}

static void removeFifo(void)
{
    fprintf(stderr, "TEST: IN REMOVE FIFO\n");
    unlink(clientFifo);
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