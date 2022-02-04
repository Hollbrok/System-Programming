#ifndef COMMUN_H_INC
#define COMMUN_H_INC

#define DEBUG_REGIME 1
#define NEED_SLEEP   0
#define NEED_LINE    1


#define DEBPRINT(args...)                       \
    if(DEBUG_REGIME)                            \
    {                                           \
        if (NEED_LINE)                          \
            fprintf(stderr, "|   LINE: %d\n"    \
                            "->", __LINE__);    \
        fprintf(stderr, args);                  \
    }

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


/* client FIFO name template */

const char CLIENT_FIFO_TEMPLATE[] = "./fifos/clientfifo.%ld";                       /* template for building client FIFO name */

/* client FIFO name length */

const int CLIENT_FIFO_NAME_LEN    = (sizeof(CLIENT_FIFO_TEMPLATE) + 15);            /* size required for client FIFO pathname */

/* well-known name of server fifo */

const char SERVER_FIFO_ACCESS[]   = "./fifos/serverfifoACCESS";                     /* this fifo using to get permission on write */

const int BUF_SIZE = PIPE_BUF;

const int TRUE  = 1;
const int FALSE = 0;


/* request to server about using FIFO  */

struct AccReq 
{
    pid_t pid;
};

/* request which consists of data from file */

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
