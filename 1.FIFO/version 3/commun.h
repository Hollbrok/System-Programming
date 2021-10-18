#ifndef COMMUN_H_INC
#define COMMUN_H_INC

#define DEBUG_REGIME 1
#define NEED_SLEEP   0

#define DEBPRINT(args...)   \
    if(DEBUG_REGIME)        \
        fprintf(stderr, args);

#define ERRCHECK_CLOSE(FD)      \
    if (close(FD) != 0)         \
    {                           \
        perror("Close #FD");    \
    }    

#define DEB_SLEEP(x, msg)                               \
    if(NEED_SLEEP)                                    \
    {                                                   \
        fprintf(stderr, msg);                           \
        fprintf(stderr, "\n\nbefore test sleep = %d\n", x);   \
        sleep(x);                                       \
        fprintf(stderr, "after test sleep = %d\n\n", x);  \
    }


/* name templates */

const char CLIENT_FIFO_ACCESS_TEMPLATE[] = "./fifos/clientfifoACCESS.%ld";        /* template for building client FIFO name */

const char CLIENT_FIFO_TEMPLATE[] = "./fifos/clientfifo.%ld";                     /* template for building client FIFO name */

/* FIFO names length */

const int  CLIENT_FIFO_NAME_LEN  = (sizeof(CLIENT_FIFO_TEMPLATE) + 15);                 /* size required for client FIFO pathname */

const int  CLIENT_FIFO_ACCESS_NAME_LEN  = (sizeof(CLIENT_FIFO_ACCESS_TEMPLATE) + 15);   /* size required for client FIFO pathname */


/* well-known name of server fifo */

const char SERVER_FIFO_ACCESS[] = "./fifos/serverfifoACCESS";                     /* this fifo using to get permission on write */


const int BUF_SIZE = PIPE_BUF;

const int TRUE  = 1;
const int FALSE = 0;

struct AccReq  // request to server about using FIFO  
{
    pid_t pid;
};

struct Accresp // answer from server about access
{
    int isYes;
};

struct Req
{
    char buffer[BUF_SIZE];
};

enum ERRORS_SPEC
{
    MKFIFO_NO_EEXIT         ,
    OPEN_FIX_FD             ,
    SIGPIPE_IGN_ERROR       ,
    WRITE_TO_CLIENT         ,
    ERROR_OPEN_TO_READ_CLIENT,
    FIFO_HANDLER_SIGNALS    ,
    SET_SIGHANDLER_ERROR    ,
    NOT_ENOUGH_ARGUMENTS    ,
    TO_MUCH_ARGUMENTS       ,
    ZEROSTRING_ARGV         ,

};

#endif
