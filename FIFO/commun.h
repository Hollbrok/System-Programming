#ifndef COMMUN_H_INC
#define COMMUN_H_INC

const char SERVER_FIFO[] = "./serverfifo";                /* Well-known name for server's FIFO */

const int BUF_SIZE = 4096;

const int TRUE  = 1;
const int FALSE = 0;

struct Req
{
    int realSize = 0;
    int NOaccess = 0;
    pid_t pid = 0;
    char buffer[BUF_SIZE] = {};
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