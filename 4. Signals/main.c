#include "libs.h"
#include "debug.h"
#include "common.h"

char klastBit;
int kchildPid = 0;
clock_t kcBegin;
long kfileSize;


void exitStuff()
{
    if (kchildPid)
        kill(kchildPid, SIGKILL);

    if (kchildPid) /* only Parent print info*/
    {  
        clock_t cEnd = clock();
        double timeSec = (double)(cEnd - kcBegin) / CLOCKS_PER_SEC;

        fprintf(stderr, "time = %lg s\n"
                    "filesize = %ld B\n"
                    "speed of transmittion = %lg kB/s\n", timeSec, kfileSize, kfileSize / (timeSec * 1000));
    }

    DEBPRINT("TEST exitStuff\n");
}

void handler0(int signum)
{
    if (kchildPid)
        DEBPRINT("parent got 0 bit\n");
    else
        DEBPRINT("\t child got response\n");
    klastBit = 0;
}

void handler1(int signum)
{
    if (kchildPid)
        DEBPRINT("got 1 bit\n");
    klastBit = 1;
}

void handlerSigChld(int signum)
{
    DEBPRINT("Child died\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
        err(EX_USAGE, "program need name-of-file argument");

    fprintf(stderr, "PID = %ld\n", (long) getpid()); /* to know PID */

    int isParent = 1;
    int parentPid = getpid();

    atexit(&exitStuff);

/* file stuff */

    int fileRD;
    if((fileRD = open(argv[1], O_RDONLY)) == -1)
        err(EX_OSERR, "open %s", argv[1]);

    kfileSize = lseek(fileRD, 0L, SEEK_END);
    
    lseek(fileRD, 0L, SEEK_SET);

/* setting handlers for SIGUSR1/2*/

    struct sigaction sa;

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
        
/* blockind signal for critical section processing */

    sigset_t prevMask, blockMask;
    sigfillset(&blockMask);

    if (sigprocmask(SIG_BLOCK, &blockMask, &prevMask) == -1)
        err(EX_OSERR ,"sigprocmast");
    

/* creating child */

    kchildPid = fork();
    switch (kchildPid)
    {
    case -1:
        ERR_HANDLER("fork()");
        break;
    case 0:
        DEBPRINT("CHILD\n");
        fprintf(stderr, "Child: %ld\n", (long) getpid());
        isParent = 0;
        break;
    default:
        DEBPRINT("PARENT\n");
        break;
    }

/* send and receive info by bits throw signals */

    kcBegin = clock();


    if (isParent)
    {
        /* */

        unsigned char byte;

        while (1) /* or while != EOF */
        {
            byte = 0;
            for(int i = 0; i < 8; ++i)
            {      
                sigsuspend(&prevMask); //pause();

                byte += klastBit << i;
                //fprintf(stderr, "byte = %d\n", byte);

                if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1)
                    err(EX_OSERR ,"sigprocmask");
                
                if (kill(kchildPid, SIGUSR1) == -1)
                    err(EX_OSERR, "kill(child, USR1)");
                
                
            }

            write(STDOUT_FILENO, &byte, 1);
        }
    }
    else /* CHILD */
    {
        int ppid = getppid();
    
        unsigned char lastByte;

        while(read(fileRD, &lastByte, 1) > 0) /* also can transmitt EOF (to indificate the end of transmittion) */
        {
            for(int i = 0; i < 8; ++i) /* оптимизация: если два подряд бита идут разные, то посылать их двое сразу, а потом ждат ответ, а не по-отдельности*/
            {
                if ( ( (unsigned char) (lastByte << (sizeof(unsigned char) * 8 - 1 - i)) ) >> (sizeof(unsigned char) * 8 - 1)) /* bit == 0b1*/
                {
                    if (kill(ppid, SIGUSR2) == -1)
                        err(EX_OSERR, "kill(parent, USR2)");
                }
                else /* bit == 0b0*/
                {
                    if (kill(ppid, SIGUSR1) == -1)
                        err(EX_OSERR, "kill(parent, USR1)");
                }

                /* wait for response */

                sigsuspend(&prevMask);

                if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1)
                    err(EX_OSERR ,"sigprocmask");
            }
        }
    }
    
    DEBPRINT("SUCCESS\n");
    exit(EXIT_SUCCESS);
}