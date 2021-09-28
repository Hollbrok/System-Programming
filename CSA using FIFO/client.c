 #include "libs.h"

//////////////////////

#define SERVER_FIFO "/tmp/seqnum_sv"            /* Well-known name for server's FIFO */

#define CLIENT_FIFO_TEMPLATE "/tmp/seqnum_cl.%ld"   /* Template for building client FIFO name */


#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)    /* Space required for client FIFO pathname
                                                                    (+20 as a generous allowance for the PID) */

static char clientFifo[CLIENT_FIFO_NAME_LEN];
const int BUF_SIZE = 512;

struct request              /* Request (client --> server) */       
{                 
    pid_t pid;              /* PID of client */
    char filename[20];      /* input number from cmd line */ 
};



struct response             /* Response (server --> client) */
{   
    char buffer[BUF_SIZE];
};



/// ALL INFO AT THE TOP CAN BE moved to a separate file (CSA_info) 

 /* Invoked on exit to delete client FIFO */

static void removeFifo(void)
{
    unlink(clientFifo);
}


//////////////////////

enum ERRORS_HANDLER
{
    NOT_ENOUGH_ARGUMENTS = -5,
    TO_MUCH_ARGUMENTS       ,
    ZEROSTRING_ARGV         ,
    MKFIFO_NO_EEXIT            ,
    OPEN_FIX_FD             ,
    SIGPIPE_IGN_ERROR       ,
    WRITE_TO_CLIENT         ,
};

long getNumber(const char *numString);

int main(int argc, const char *argv[])
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

/// 

    int serverWFd = -1;      // write request
    int clientRFd = -1;      // read server response

    struct request req;     // request to server from client
    struct response resp;   // answer to client from server

///

    /* Create our FIFO (before sending request, to avoid a race) */

    umask(0); /* So we get the permissions we want */
    
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

    /* Construct request message, open server FIFO, and send request */
    req.pid = getpid();
    strncpy(req.filename, argv[1], 20);
    //req.inNum = getNumber(argv[1]);

    serverWFd = open(SERVER_FIFO, O_WRONLY);

    if (serverWFd == -1)
    {
        fprintf(stderr, "ERROR in open on write server FIFO\n");
        exit(EXIT_FAILURE); // add special error-name
    }


    if ( write(serverWFd, &req, sizeof(struct request)) != sizeof(struct request) )
    {
        fprintf(stderr, "Cant write to server FIFO\n");
        exit(EXIT_FAILURE); // add special error-name
    }

    /* Open our FIFO, read and display response */

    clientRFd = open(clientFifo, O_RDONLY);
    if (clientRFd == -1)
    {
        fprintf(stderr, "can't open clientFIFO to read\n");
        exit(EXIT_FAILURE); // add special error-name
    }
    
    int NOread;
    //int NOiterations = 0;
    while ( (NOread = read(clientRFd, &resp, sizeof(struct response)) ) > 0 )
    {
        //if (NOread  < sizeof(struct response))
        //    resp.buffer[NOread] = '\0';

        fprintf(stderr, "[%d]Read info:\n[%.*s]\n", NOread, NOread, resp.buffer);
        //NOiterations++;
        //fprintf(stderr, "Iteration %d\n", NOiterations);
    }

    /*if ( (NOread = read(clientRFd, &resp, sizeof(struct response)) ) <= 0 )//( (NOread = read(clientRFd, &resp, sizeof(struct response)) ) != sizeof(struct response) )
    {
        fprintf(stderr, "Can't get answer from server. NOread = %d (need = %d)\n", NOread, sizeof(struct response));
        exit(EXIT_FAILURE); // add special error-name
    }*/

    //printf("sum = %d\n", resp.sum);
    //printf("NO appeals = %d\n", resp.NOappeal);   
    //fprintf(stdout, "Read info:\n", resp.buffer);

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
