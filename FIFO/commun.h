#ifndef COMMUN_H_INC
#define COMMUN_H_INC


#define DEBUG_REGIME TRUE

#define DEBPRINT(args...)   \
    if(DEBUG_REGIME)        \
        fprintf(stderr, args);

#define ERRCHECK_CLOSE(FD)      \
    if (close(FD) != 0)         \
    {                           \
        perror("Close #FD");    \
    }    

const char CLIENT_FIFO_TEMPLATE[] = "./clientfifo.%ld";   /* Template for building client FIFO name */

const int  CLIENT_FIFO_NAME_LEN  = (sizeof(CLIENT_FIFO_TEMPLATE) + 20);    /* Space required for client FIFO pathname */

const char SERVER_FIFO_ACCESS[] = "./serverfifoACCESS";   /* this fifo using to get permission on write */

const char SERVER_FIFO[] = "./serverfifo";                /* Well-known name for server's FIFO */

const int BUF_SIZE = 4096;

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

enum ERRORS_HANDLER
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