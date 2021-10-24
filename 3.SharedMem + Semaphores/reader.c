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

    /* get already exists sem's and shm + attach shm */

    if ( (semId = semget(SEM_KEY, 2,  IPC_CREAT | IPC_EXCL | OBJ_PERMS)) == -1) /* if already exists or EXCL creation*/
    {
        DEBPRINT("can't create EXCL sem\n")
        
        if (errno != EEXIST) /* Unexpected error from semget() */
        {
            LEAVE_STUFF
            ERR_HANDLER("semget")
        }

        semId = semget(SEM_KEY, 0, 0);
        
        DEBPRINT("semId = %d\n", semId)

        if (semId == -1)
        {
            LEAVE_STUFF
            ERR_HANDLER("semget")
        }
    }
    else
    {
        DEBPRINT("sem created EXCL [id = %d]\n Initialization\n", semId)

        if (initSem(semId, SEM_W, AvailableToUse) == -1)
        {
            LEAVE_STUFF
            ERR_HANDLER("initSem (AVB)")
        }
        DEBPRINT("after initialization WRITE sem (AVB)\n")

        if (initSem(semId, SEM_R, InUse) == -1)
        {
            LEAVE_STUFF
            ERR_HANDLER("initSem (InUse)")
        }

        DEBPRINT("after initialization READ sem (InUse)\n")
    }

    DEBPRINT("after initialization all sems\n")

    errno = 0;

    /* allocate a System V shared memory segment and attach it */

    if ( (shmId = shmget(SHM_KEY, sizeof(struct ShmSeg), IPC_CREAT | IPC_EXCL | OBJ_PERMS)) == -1)
    {
        DEBPRINT("can't create EXCL shm\n")

        if (errno != EEXIST) /* Unexpected error from semget() */
        {
            LEAVE_STUFF
            ERR_HANDLER("shmget")
        }

        shmId = shmget(SHM_KEY, 0, 0);
        
        DEBPRINT("shmId = %d\n", shmId)

        if (shmId == -1)
        {
            LEAVE_STUFF
            ERR_HANDLER("shmget")
        }
    }
    else
    {
        DEBPRINT("shm created EXCL\n")
    }

    if ( (shmSeg = shmat(shmId, NULL, 0)) == (void *) -1)
    {
        LEAVE_STUFF
        ERR_HANDLER("shmat")
    }
    DEBPRINT("successful shmat\n")


    /* get info in a loop */

    while (1)
    {
        if (reserveSem(semId, SEM_R) == -1)
        {
            if (errno == EAGAIN)
            {
                LEAVE_STUFF
                ERR_HANDLER("EAGAIN")
            }
            else
            {
                LEAVE_STUFF
                ERR_HANDLER("reserve READ sem (errno !=EINTR)")
            }
        } 

        DEBPRINT("after reserve\n")

        if (shmSeg->cnt == 0)
            break;

        fprintf(stderr, "[%.*s][%d]", shmSeg->cnt, shmSeg->buf, shmSeg->cnt);


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

