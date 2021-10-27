/* READER */


#include "libs.h"
#include "common.h"
#include "debug.h"


int main(int argc, char* argv[])
{
    /* data section */
    
    int semId, shmId;
    int lastByteRead;

    struct ShmSeg *shmSeg;
    union semun uselessArg;

    /* get (create) a System V semaphore set identifier and initialize them*/

    semId = semGet(READER);

    DEBPRINT("after initialization all sems\n")

    errno = 0; 

    /* allocate a System V shared memory segment and attach it */

   shmId = shmGet();

    if ( (shmSeg = shmat(shmId, NULL, 0)) == (void *) -1)
    {
        LEAVE_STUFF
        ERR_HANDLER("shmat")
    }

    DEBPRINT("successful shmat\n")


    /* get info in a loop */

    if (reserveSem(semId, SEM_W_INIT) == -1)
    {
        LEAVE_STUFF
        ERR_HANDLER("reserve SEM_W_INIT")
    }

    if (DEBUG_REGIME)
        printSem(semId);

    //DEBPRINT("TEST: E_VAL = %d\n", getSemVal(semId, SEM_E))

    while (1)
    {
        if (reserveSem(semId, SEM_R) == -1)
        {
            LEAVE_STUFF
            ERR_HANDLER("reserve READ sem (errno !=EINTR)")
        } 

        DEBPRINT("after reserve\n")

        if (getSemVal(semId, SEM_E) == 1)
        {
            LEAVE_STUFF
            DEBPRINT("WRITER DEAD!!!!!!!!!!!\n")
            exit(EXIT_FAILURE);
        }

        if (shmSeg->cnt == 0)
        {
            DEBPRINT("BREAK FROM WHILE(1)\n")
            break;
        }
    
        fprintf(stderr, "%.*s", shmSeg->cnt, shmSeg->buf);
        DEBPRINT("[%d]\n", shmSeg->cnt);

        if (releaseSem(semId, SEM_W) == -1)
        {
            LEAVE_STUFF
            ERR_HANDLER("release WRITE sem")
        }
    }

    /*if (shmdt(shmSeg) == -1)
    {
        perror("detach shm");
        exit(EXIT_FAILURE);
    }*/

    /* give turn to writer to use [sem/shm]ctl  and detach shm*/

    if (releaseSem(semId, SEM_W) == -1)
    {
        LEAVE_STUFF
        ERR_HANDLER("release WRITE sem")
    }

    LEAVE_STUFF


    exit(EXIT_SUCCESS);
}

