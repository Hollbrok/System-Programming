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
 
    if ( (semId = semget(SEM_KEY, NO_SEMS,  IPC_CREAT | OBJ_PERMS)) == -1) /* if already exists or EXCL creation*/
        ERR_HANDLER("semget");

    printSem(semId, "before init R");

/* Start of critical section (initialization) */
       
    /* the reader waits for the rest of readers to finish their work */

    struct sembuf checkAnotherReaders[2] = {
        {SEM_R_ALIVE, 0, 0},
        {SEM_R_ALIVE, +1, SEM_UNDO}
    };

    if (semop(semId, checkAnotherReaders, 2) == -1)
        ERR_HANDLER("Start critical section of initialization of reader\n");

    /* wait writer init */

    struct sembuf waitInitW[2] = {
        {SEM_W_INIT, -1, SEM_UNDO},
        {SEM_W_INIT, +1, 0}
    };

    if (semop(semId, waitInitW, 2) == -1)
        ERR_HANDLER("wait writer init\n");

    /* to detect death */

    struct sembuf releaseE = {SEM_E, +1, SEM_UNDO};

    if (semop(semId, &releaseE, 1) == -1)
        ERR_HANDLER("release writer sem");

    /* end of init, to make it clear to the writer that the reader has finished initialization */

        struct sembuf EndInitReader[2] = {
        {SEM_R_INIT, 0, 0},
        {SEM_R_INIT, +1, SEM_UNDO}
    };

    if (semop(semId, EndInitReader, 2) == -1)
        ERR_HANDLER("End critical section of initialization of reader\n"); 

    /* after reader die reader won't stay in block */

    struct sembuf undoReleaseW[2] = {
        {SEM_W, 1, 0},
        {SEM_W, -1, SEM_UNDO}
    };

    if (semop(semId, undoReleaseW, 2) == -1)
        ERR_HANDLER("undo release of reader");

/* end of initialization */

    fprintf(stderr, "after init\n");

    DEBPRINT("after initialization of all sems\n")

    /* allocate a System V shared memory segment and attach it */

    errno = 0; 

    if ( (shmId = shmget(SHM_KEY, sizeof(struct ShmSeg), IPC_CREAT | OBJ_PERMS)) == -1)
        ERR_HANDLER("shmget");

    if ( (shmSeg = shmat(shmId, NULL, 0)) == (void *) -1)
        ERR_HANDLER("shmat");

    DEBPRINT("before reserver W_INIT\n")

    struct sembuf reserveInitW = {SEM_W_INIT, -1, 0};

    if (semop(semId, &reserveInitW, 1) == -1)
        ERR_HANDLER("reserve SEM_W_INIT");

    if (DEBUG_REGIME)
        printSem(semId, "after reserve W_INIT\n"
                        "Before while");

    int transferErr = 0;

    printSem(semId, "before while(1)");

    while (1)
    {
        //fprintf(stderr, "1");

        if (reserveSem(semId, SEM_R) == -1)
            ERR_HANDLER("reserve READ sem");
        
        /* if another side is dead */

        if (getSemVal(semId, SEM_W_ALIVE) == 0)
        {
            if (DEBUG_REGIME)
                printSem(semId, "after death of writer");

            transferErr = 1;
            break;
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

    printSem(semId, "after while(1)");


    /* give turn to writer to use [sem/shm]ctl and detach shm*/

    struct sembuf endStuff[3] = {
        {SEM_W_INIT, +1, 0},
        {SEM_W_INIT, -1, SEM_UNDO},
        {SEM_W, +1, 0}
    };

    //{SEM_W_INIT, -1, SEM_UNDO},

    if (semop(semId, endStuff, 3) == -1)
       ERR_HANDLER("semop endStuff");

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

    fprintf(stderr, "%s\n", transferErr ? "FAILED" : "SUCCESS");

    exit(EXIT_SUCCESS);
}

