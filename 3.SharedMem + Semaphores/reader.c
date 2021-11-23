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
        ERR_HANDLER("shmat");

    DEBPRINT("successful shmat\n")


    /* get info in a loop */

    DEBPRINT("before reserver W_INIT\n")

    if (reserveSem(semId, SEM_W_INIT) == -1)
        ERR_HANDLER("reserve SEM_W_INIT");

    if (DEBUG_REGIME)
        printSem(semId, "after reserve W_INIT\n"
                        "Before while");


    while (1)
    {
        if (reserveSem(semId, SEM_R) == -1)
            ERR_HANDLER("reserve READ sem");
        
        /* if another side is dead */

        if (getSemVal(semId, SEM_E) == 1)
        {
            if (DEBUG_REGIME)
                printSem(semId, "after death of writer");

            exit(EXIT_SUCCESS);
        }

        if (shmSeg->cnt == 0)
        {
            DEBPRINT("BREAK FROM WHILE(1)\n")
            break;
        }

        write(STDOUT_FILENO, shmSeg->buf, shmSeg->cnt);
        
        DEBPRINT("%d\n", shmSeg->cnt);   

        if (releaseSem(semId, SEM_W) == -1)
            ERR_HANDLER("release WRITE sem");
    }


    /* give turn to writer to use [sem/shm]ctl and detach shm*/

    if (releaseSem(semId, SEM_W) == -1)
        ERR_HANDLER("release WRITE sem");

    if (semctl(semId, 0, IPC_RMID, NULL) == -1)        
    {                                                  
        if (errno != EINVAL)                            
            ERR_HANDLER("remove semId");                                        
    }

    if (shmdt(shmSeg) == -1)                           
        ERR_HANDLER("detach shm");  
                   
    if (shmctl(shmId, IPC_RMID, 0) == -1)
    {
        if (errno != EINVAL)           
            ERR_HANDLER("remove shm Seg");   
    }    


    exit(EXIT_SUCCESS);
}

