#include "libs.h"
#include "debug.h"
#include "common.h"


int kchildPid = 0;
char klastBit;

clock_t kcBegin;
long kfileSize;

int ksuccess = 1;


void exitStuff()
{
    if (kchildPid)
        kill(kchildPid, SIGKILL);

    if (kchildPid) /* only parent print info*/
    {  
        clock_t cEnd = clock();
        double timeSec = (double)(cEnd - kcBegin) / CLOCKS_PER_SEC;

        //fprintf(stderr, "\nStatus of transmission = %s\n", ksuccess == 1 ? "success" : "failed");
        fprintf(stderr, "transmission time      = %lg s\n"
                        "file size              = %ld B\n"
                        "speed of transmission  = %lg kB/s\n", timeSec, kfileSize, kfileSize / timeSec /  1024);
    }

    DEBPRINT("TEST exitStuff\n");
}

void handler0(int signum)
{
    if (kchildPid)
        DEBPRINT("parent got 0b0\n");
    else
        DEBPRINT("\t child got response\n");

    klastBit = 0;
}

void handler1(int signum)
{
    if (kchildPid)
        DEBPRINT("got 0b1\n");
    
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

    fprintf(stderr, "Parent: %ld\n", (long) getpid()); 

    int isParent = 1;
    int parentPid = getpid();

    atexit(&exitStuff);

/* file stuff */

    int fileRD;
    if((fileRD = open(argv[1], O_RDONLY)) == -1)
        err(EX_OSERR, "open %s", argv[1]);

    kfileSize = lseek(fileRD, 0L, SEEK_END);
    
    lseek(fileRD, 0L, SEEK_SET);

/* set handlers for SIGUSR1(2) */

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
        
/* block signal for critical section processing */

    sigset_t prevMask, blockMask;
    sigfillset(&blockMask);

    if (sigprocmask(SIG_BLOCK, &blockMask, &prevMask) == -1)
        err(EX_OSERR ,"sigprocmast");
    

/* create child */

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
        unsigned char byte;

        while (1)
        {
            byte = 0;
            for (int i = 0; i < 8; ++i)
            {      
                sigsuspend(&prevMask);                              /* unblock + get signal (atomically) */

                byte += klastBit << i;

                if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1) /* start of critical section so should block signals */
                    err(EX_OSERR ,"sigprocmask");
                
                if (kill(kchildPid, SIGUSR1) == -1)                 /* answer to child that we are ready */
                    err(EX_OSERR, "kill(child, USR1)");
            }

            write(STDOUT_FILENO, &byte, 1);
        }

        fprintf(stderr, "PARENT after while(1)\n");
    }
    else /* CHILD */
    {
        int ppid = getppid();
    
        unsigned char lastByte;

        while (read(fileRD, &lastByte, 1) > 0) 
        {
            for(int i = 0; i < 8; ++i)
            {
                if ( ( (unsigned char) (lastByte << (8 - 1 - i)) ) >> (8 - 1)) /* bit == 0b1*/
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

                sigsuspend(&prevMask);                              /* unblock + get signal (atomically) */

                if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1) /* start of critical section so should block signals */
                    err(EX_OSERR ,"sigprocmask");
            }
        }
    }
    
    exit(EXIT_SUCCESS);
}