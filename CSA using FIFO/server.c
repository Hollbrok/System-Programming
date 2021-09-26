 #include "libs.h"

//////////////////////

#define SERVER_FIFO "/tmp/seqnum_sv"                /* Well-known name for server's FIFO */

#define SERVER_BACKUP_FILE "/tmp/backup"            /* backup files for save different info */

#define CLIENT_FIFO_TEMPLATE "/tmp/seqnum_cl.%ld"   /* Template for building client FIFO name */

#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)    /* Space required for client FIFO pathname
                                                                    (+20 as a generous allowance for the PID) */

const int TRUE  = 1;
const int FALSE = 0;

static int NO_APPEALS; 

struct request      /* Request (client --> server) */       
{                 
    pid_t pid;      /* PID of client */
    int inNum;      /* input number from cmd line */ 
};



struct response     /* Response (server --> client) */
{   
    int sum;        
    int NOappeals;
};



//////////////////////

enum ERRORS_HANDLER
{
    MKFIFO_NO_EEXIT         ,
    OPEN_FIX_FD             ,
    SIGPIPE_IGN_ERROR       ,
    WRITE_TO_CLIENT         ,
    ERROR_OPEN_TO_READ_CLIENT,
    FIFO_HANDLER_SIGNALS    ,
    SET_SIGHANDLER_ERROR    ,

};

static long getNumber(const char *numString);

static void handlerFIFO(int sig);

static void updateBackUpFile(void);

static void setSignalsHandler();

static int getBackupData(); 

static void fixFifoEof();

static void createServerFIFO();

static int getRDofFIFO();

int main(int argc, const char *argv[])
{
    int serverRFd = -1;             /* read client request              */
    int clientFd = -1;              /* read server response             */

    struct request req   = {};      /* request to server from client    */
    struct response resp = {};      /* answer to client from server     */

    char clientFifo[CLIENT_FIFO_NAME_LEN];

/* Set signals (INT + TERM) handlers                            */

    setSignalsHandler();

/* Get data from backup file (Number of appeals)                */

    resp.NOappeals = getBackupData();
    NO_APPEALS = resp.NOappeals;


/* Creating FIFO (only 1 for server to read from all clients)   */

    createServerFIFO();


/* fix that fifo may meet EOF if there are no clients           */

    fixFifoEof();
    
/* Get FD to read from serverFIFO                               */

    serverRFd = getRDofFIFO();

/* Getting Request from client                                  */

    while (TRUE)
    {
        /* trying to read request from client until we got it                                           */

        if (read(serverRFd, &req, sizeof(struct request)) != sizeof(struct request))
        {
            fprintf(stderr, "Not full read from client or error, continue to read.\n");
            continue;
        }

        /* generate unique name for (client-read from server) FIFO */

        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) req.pid); 
        
        /* get FD of exclusive client FIFO (see the pic in README) to answer to client                  */

        clientFd = open(clientFifo, O_WRONLY); /* Open failed, give up on client, reading the next data */
        if (clientFd == -1)         
        {
            fprintf(stderr, "client FIFO failed to open."); 
            continue;
        }

        /* Now server can send response to client by client FIFO                                        */

        resp.sum = req.inNum + req.pid;
        resp.NOappeals += 1;

        NO_APPEALS = resp.NOappeals;

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

static long getNumber(const char *numString)
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

static void handlerFIFO(int sig)
{
    unlink(SERVER_FIFO);

    fprintf(stderr, "FIFO handler got SIGNAL: %s\nTERMINATE\n", strsignal(sig));

    exit(FIFO_HANDLER_SIGNALS);
}

static void updateBackUpFile(void)
{
    // Update NO_APPEALS -- global const

    int backupFd;
    if ( (backupFd = open(SERVER_BACKUP_FILE, O_SYNC | O_RDWR)) == -1) 
    {
        if ( (backupFd = open(SERVER_BACKUP_FILE, O_SYNC | O_RDWR | O_CREAT, S_IRWXU)) == -1)
        {
            perror("Can't create backup file");
            return;
        }
    }

    if( lseek(backupFd, 0, SEEK_SET) == -1)
    {
        perror("lseek");
        return;
    }

    if(write(backupFd, &NO_APPEALS, sizeof(int)) != sizeof(int))
    {
        perror("ERROR in updating backup file");
        return;
    }

    if( close(backupFd) == -1)
    {
        perror("ERROR in close backup file");
    }

}

static void setSignalsHandler()
{
    if ( (signal(SIGINT, handlerFIFO) == SIG_ERR) || (signal(SIGTERM, handlerFIFO) == SIG_ERR) )
    {
        fprintf(stderr, "Can't set signal handler for SIGINT or SIGTERM\n");
        exit(SET_SIGHANDLER_ERROR);
    }
}

static int getBackupData()
{
    int backupFd;
    if ( (backupFd = open(SERVER_BACKUP_FILE, O_SYNC | O_RDWR)) == -1) 
    {
        if ( (backupFd = open(SERVER_BACKUP_FILE, O_SYNC | O_RDWR | O_CREAT, S_IRWXU)) == -1)
        {
            perror("Can't create backup file");
            exit(EXIT_FAILURE);
        }
    }

    if (atexit(updateBackUpFile) != 0)
    {
        fprintf(stderr, "Can't set exit function\n");
        exit(EXIT_FAILURE);
    }  
    

    struct stat st  = {}; 
    int backupFSize = 0;

    if (stat(SERVER_BACKUP_FILE, &st) == 0)
        backupFSize = st.st_size;

    int NOappeals = 0;

    if (backupFSize == 0)
    {
        fprintf(stderr, "TEST: backupFSize = 0\n");
        return NOappeals;
    }
    else
    {
        fprintf(stderr, "TEST: backupFSize = %d\n", backupFSize);
        if( read(backupFd, &NOappeals, sizeof(int)) != sizeof(int))
        {
            perror("Can't read data from backup file");
            exit(EXIT_FAILURE); // or just = 0, not EXIT
        }
        fprintf(stderr, "TEST: read resp.NOappeals = %d\n", NOappeals);
    }

    return NOappeals;
}

static void fixFifoEof()
{
    int fixFd  = -1;    /* this is FD to fix problem with FIFO: it needs to open FIFO write end to not meet EOF. */

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
}

static void createServerFIFO()
{
    int mkfifoStatus = mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP);

    while (mkfifoStatus == -1)
    {
        perror("\nTEST: MKFIFO returns -1\nTEST: errno");

        if( unlink(SERVER_FIFO) == -1)
        {
            perror("Can't unlink FIFO, exit\n");
            exit(EXIT_FAILURE);
        }

        if(errno != EEXIST)
        {
            fprintf(stderr, "ERROR: mk(client)fifo. ERROR IS NOT EEXIST\n");
            exit(MKFIFO_NO_EEXIT);
        } 

        mkfifoStatus = mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP);
    }

}

static int getRDofFIFO()
{
    int serverRFd = open(SERVER_FIFO, O_RDONLY);
    if (serverRFd == -1)
    {
        fprintf(stderr, "ERRORopen %s", SERVER_FIFO);
        exit(ERROR_OPEN_TO_READ_CLIENT);
    }

    return serverRFd;
}