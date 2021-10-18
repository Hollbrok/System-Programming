#ifndef COMMUN_H_INC
#define COMMUN_H_INC

#ifdef ND

#define DEBUG_REGIME 1

#else

#define DEBUG_REGIME 0

#endif



#define NEED_SLEEP   0
#define NEED_LINE    1


#define DEBPRINT(args...)                       \
    if(DEBUG_REGIME)                            \
    {                                           \
        if (NEED_LINE)                          \
            fprintf(stderr, "\n|                LINE: %d\n"    \
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


/* server FIFO name template */

const char SERVER_FIFO_TEMPLATE[] = "./fifos/serverfifo.%ld";                       /* template for building client FIFO name */

/* server FIFO name length */

const int SERVER_FIFO_NAME_LEN    = (sizeof(SERVER_FIFO_TEMPLATE) + 15);            /* size required for client FIFO pathname */

/* well-known name of server fifo */

const char SERVER_FIFO_ACCESS[]   = "./fifos/serverfifoACCESS";                     /* this fifo using to get permission on write */

const int BUF_SIZE = PIPE_BUF;

const int TRUE  = 1;
const int FALSE = 0;


/* data which writing to server ACCESS FIFO about using FIFO  */

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
