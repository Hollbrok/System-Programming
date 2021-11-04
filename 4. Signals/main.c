#include "libs.h"
#include "debug.h"
#include "common.h"

char klastBit;
int kchildPid = 0;

void exitStuff()
{
    if (kchildPid)
        kill(kchildPid, SIGKILL);
    fprintf(stderr, "TEST\n");
}

void handler0(int signum)
{
    if (kchildPid)
        fprintf(stderr, "got 0 bit\n");
    else
        fprintf(stderr, "\t child got response\n");
    klastBit = 0;
}

void handler1(int signum)
{
    if (kchildPid)
        fprintf(stderr, "got 1 bit\n");
    klastBit = 1;
}

void handlerSigChld(int signum)
{
    printf("Child died\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
        err(EX_USAGE, "program need name-of-file argument");

    fprintf(stderr, "PID = %ld\n", (long) getpid()); /* to know PID */

    int isParent = 1;
    int parentPid = getpid();
        
/* initialization stuff */

    atexit(&exitStuff);

    sigset_t prevMask, intMask;
    struct sigaction sa;

    sigemptyset(&intMask);
    sigaddset(&intMask, SIGUSR1);

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sa.sa_handler = handler0;
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        err(EX_OSERR, "sigaction to SIGUSR1");

    sa.sa_handler = handler1;
    if (sigaction(SIGUSR2, &sa, NULL) == -1)
        err(EX_OSERR, "sigaction to SIGUSR2");

    sa.sa_handler = handlerSigChld;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        err(EX_OSERR, "sigaction to SIGCHLD");

/* creating child */

    kchildPid = fork();
    switch (kchildPid)
    {
    case -1:
        ERR_HANDLER("fork()");
        break;
    case 0:
        DEBPRINT("CHILD\n");
        isParent = 0;
        break;
    default:
        DEBPRINT("PARENT\n");
        break;
    }

/* send and receive info by bits throw signals */
/* 00110001 */
    if (isParent)
    {
        /* */
        fprintf(stderr, "\t\t\tCHLD === %d\n", kchildPid);

        unsigned char byte;

        while (1) /* or while != EOF */
        {
            byte = 0;
            for(int i = 0; i < 8; ++i)
            {
                fprintf(stderr, "i =%d\n", i);
                fprintf(stderr, "P Before pause\n");
                pause();
                fprintf(stderr, "P After pause\n\n\n");

                byte += klastBit << i;
                fprintf(stderr, "cur byte = %d\n", byte);


                if (kill(kchildPid, SIGUSR1) == -1)
                    err(EX_OSERR, "kill(child, USR1)");
            }

            fprintf(stderr, "[%c][%d]", byte, byte);//write(STDOUT_FILENO, &byte, 1);
        }
    }
    else /* CHILD */
    {
        int ppid = getppid();
        fprintf(stderr, "\t\t\tPPID === %d\n", ppid);

        int fileRD;
        if((fileRD = open(argv[1], O_RDONLY)) == -1)
            err(EX_OSERR, "open %s", argv[1]);
    
        unsigned char lastByte;

        /* first */

        while(read(fileRD, &lastByte, 1) > 0) /* also can transmitt EOF (to indificate the end of transmittion) */
        {
            /*unsigned char x = lastByte;
            if (x == 0)
                fprintf(stderr, "x == 0\n");
            else
            {
                fprintf(stderr, "x = %d\n", x);
                for (int i = 0; i < 8; i++)
                {
                    fprintf(stderr, "[%d]", ( (unsigned char) (x << (sizeof(unsigned char) * 8 - 1 - i)) ) >> (sizeof(unsigned char) * 8 - 1));
                }
            }*/
            for(int i = 0; i < 8; ++i) /* оптимизация: если два подряд бита идут разные, то посылать их двое сразу, а потом ждат ответ, а не по-отдельности*/
            {
                //fprintf(stderr, "\ti =%d\n", i);
                if ( ( (unsigned char) (lastByte << (sizeof(unsigned char) * 8 - 1 - i)) ) >> (sizeof(unsigned char) * 8 - 1)) /* bit == 0b1*/
                {
                    if (kill(ppid, SIGUSR2) == -1)
                        err(EX_OSERR, "kill(parent, USR2)");
                    fprintf(stderr, "\t [1]\n");
                }
                else /* bit == 0b0*/
                {
                    if (kill(ppid, SIGUSR1) == -1)
                        err(EX_OSERR, "kill(parent, USR1)");
                    fprintf(stderr, "\t [0]\n");
                }
                /* wait for response */
                fprintf(stderr, "\tC before pause\n");
                pause();
                fprintf(stderr, "\tC after pause\n");
            }
        }
    }
    
    DEBPRINT("SUCCESS\n")
    exit(EXIT_SUCCESS);
}