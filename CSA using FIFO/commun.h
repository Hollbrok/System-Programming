#ifndef COMMUN_H_INC
#define COMMUN_H_INC

const char SERVER_FIFO[] = "/tmp/seqnum_sv";                /* Well-known name for server's FIFO */

const char CLIENT_FIFO_TEMPLATE[] = "/tmp/seqnum_cl.%ld";   /* Template for building client FIFO name */

const int  CLIENT_FIFO_NAME_LEN  = (sizeof(CLIENT_FIFO_TEMPLATE) + 20);    /* Space required for client FIFO pathname
                                                                            (+20 as a generous allowance for the PID) */

const int BUF_SIZE = 4096;

const int TRUE  = 1;
const int FALSE = 0;

struct request              /* Request (client --> server) */       
{                 
    pid_t pid;              /* PID of client */
    char filename[20];      /* name of the file from cmd line */ 
};

struct response             /* Response (server --> client) */
{   
    char buffer[BUF_SIZE];  /* Buffer for reading blocks of bytes from file */
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