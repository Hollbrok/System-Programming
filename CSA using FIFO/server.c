 #include "libs.h"

//////////////////////

#define SERVER_FIFO "/tmp/seqnum_sv"            /* Well-known name for server's FIFO */

#define CLIENT_FIFO_TEMPLATE "/tmp/seqnum_cl.%ld"   /* Template for building client FIFO name */


#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)    /* Space required for client FIFO pathname
                                                                    (+20 as a generous allowance for the PID) */

struct request      /* Request (client --> server) */       
{                 
    pid_t pid;      /* PID of client */
    int inNum;     /* Length of desired sequence */ 
};



struct response     /* Response (server --> client) */
{   
    int sum;     /* Start of sequence */
    int NOappeal;
};



//////////////////////

enum ERRORS_HANDLER
{
    NOT_ENOUGH_ARGUMENTS = -5,
    TO_MUCH_ARGUMENTS       ,
    ZEROSTRING_ARGV         ,
    MKFIFO_NO_EEXIT         ,
    OPEN_FIX_FD             ,
    SIGPIPE_IGN_ERROR       ,
    WRITE_TO_CLIENT         ,
    ERROR_OPEN_TO_READ_CLIENT,
};

long getNumber(const char *numString);

int main(int argc, const char *argv[])
{
    /*
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
    */

/// 

    int serverFd = -1;      // read client request
    int clientFd = -1;      // read server response

    int fixFd  = -1;        // this is FD to fix problem with FIFO: it needs to open FIFO write end to not meet EOF.

    struct request req;     // request to server from client
    struct response resp;   // answer to client from server

    char clientFifo[CLIENT_FIFO_NAME_LEN];

//

    // Creating FIFO (only 1 for server to read from all clients)

    umask(0);       /* So we get the permissions we want */
    errno = 0;

    while (mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
    {
        fprintf(stderr, "\nTEST: MKFIFO returns -1\n");
        perror("TEST: errno?\n");

        unlink(SERVER_FIFO);

        if(errno != EEXIST)
        {
            fprintf(stderr, "ERROR: mk(client)fifo. ERROR IS NOT EEXIST\n");
            exit(MKFIFO_NO_EEXIT);
        } 
    }
    
    /*if (mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1 
        && errno != EEXIST)
    {
        fprintf(stderr, "ERROR: mk(client)fifo. ERROR IS NOT EEXIST\n");
        exit(MKFIFO_NO_EEXIT);
    } */



    serverFd = open(SERVER_FIFO, O_RDONLY);
    if (serverFd == -1)
    {
        fprintf(stderr, "ERRORopen %s", SERVER_FIFO);
        exit(ERROR_OPEN_TO_READ_CLIENT);
    }

    // fix EOF stuff

    fixFd = open(SERVER_FIFO, O_WRONLY);

    if (fixFd  == -1)
    {
        fprintf(stderr, "ERROR: open fixFD. %s\n", SERVER_FIFO);
        exit(OPEN_FIX_FD);
    }
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        fprintf(stderr, "Can't set ignore to do in signal SISPIPE.\n");
        exit(SIGPIPE_IGN_ERROR);
    }
    

    // Getting REQUEST AND SENDING RESPONSE
    resp.NOappeal = 0;

    while(1)
    {
        // trying to read request from client until we read it  

        if(read(serverFd, &req, sizeof(struct request)) != sizeof(struct request))
        {
            fprintf(stderr, "Not full read from client or error, continue to read.\n");
            continue;
        }

        // get FD of exclusive client FIFO (see the pic in README) to answer him

        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) req.pid); // get unique name for client-read from server FIFO
        
        clientFd = open(clientFifo, O_WRONLY);
        if (clientFd == -1)         /* Open failed, give up on client */
        {
            fprintf(stderr, "client FIFO failed to open."); 
            continue;
        }

        /// Now server can send response to client by client FIFO

        resp.sum = req.inNum + req.pid;
        resp.NOappeal += 1;

        if ( write(clientFd, &resp, sizeof(struct response)) != sizeof(struct response) )
        {
            fprintf(stderr, "Error in write to client\n");
            exit(WRITE_TO_CLIENT);
        }

        printf("SERVER: size of write response = %d\n", sizeof(struct response));

        if ( close(clientFd) != 0 )
        {
            fprintf(stderr, "Error in close client FD\n");
        }


    }
    ///

///
    exit(EXIT_SUCCESS);
}

long getNumber(const char *numString)
{
    if (*numString == '\0')
    {
        fprintf(stderr, "empty number argument\n");
        exit(EXIT_FAILURE);
    }

    errno = 0;

    long gNumber;
    char* endOfEnter;

    const int baseOfNumber = 10;
    gNumber = strtol(numString, &endOfEnter, baseOfNumber);

    if(*endOfEnter != '\0')
    {
        fprintf(stderr, "strtol error\n");
        exit(EXIT_FAILURE);
    }
    if (errno != 0)
    {
        fprintf(stderr, "strtol error\n");
        exit(EXIT_FAILURE);
    }
    
    return gNumber;

}
