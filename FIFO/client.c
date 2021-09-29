#include "libs.h"
#include "commun.h"


/* on succ-exit must delete client FIFO */

static void removeFifo(void);

static long getNumber(const char *numString);

int main(int argc, const char *argv[])
{

/* TEST STUFF */

    //sleep(5);

/* */

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

    struct Req req;

    for (int i = 0; i < BUF_SIZE; i++)
        req.buffer[i] = 'x';
    req.buffer[BUF_SIZE - 1] = '\0';

    int serverWFd = -1;      // write request

    serverWFd = open(SERVER_FIFO, O_WRONLY); // a lot of client can open FIFO

    if (serverWFd == -1)
    {
        fprintf(stderr, "ERROR in open on write server FIFO\n");
        exit(EXIT_FAILURE); // add special error-name
    }

    int fileRD;
    if ( (fileRD = open(argv[1], O_RDONLY)) == -1)
    {
        perror("Can't open file on read\n");
        exit(EXIT_FAILURE);
    }

    int lastByteRead;

    req.NOaccess = 0;

    while ( (lastByteRead = read(fileRD, req.buffer, BUF_SIZE)) > 0 )
    {
        req.realSize = lastByteRead;
        req.pid = getpid();

        if ( write(serverWFd, &req, BUF_SIZE + 2 * sizeof(int) + sizeof(pid_t)) != (BUF_SIZE + 2 * sizeof(int) + sizeof(pid_t)) )
        {
            fprintf(stderr, "Can't write to server FIFO\n");
            exit(EXIT_FAILURE); // add special error-name
        }
        ++req.NOaccess;
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

    fprintf(stderr, "SECCUSSFUL\n");

    exit(EXIT_SUCCESS);
}

static void removeFifo(void)
{
    fprintf(stderr, "TEST: IN REMOVE FIFO\n");
    unlink(SERVER_FIFO);
}
