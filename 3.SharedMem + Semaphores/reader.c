/* READER */

 
#include "libs.h"
#include "common.h"
#include "debug.h"
 

int main(int argc, char* argv[])
{
    /* data section */
    
    int semId = -1, shmId = -1; /* IPS IDs                    */
    int lastByteRead;   /* file data transfering stuff  */

    struct ShmSeg * shmSeg = NULL;
    union semun uselessArg;
    int transferErr = 0;

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

    /* wait 0 pairs (0 processes) in executing */

    struct sembuf wait0Proc = {SEM_E, 0, 0};

    if (semop(semId, &wait0Proc, 1) == -1)
        ERR_HANDLER("wait 0 processes");

    /* cycle of file tranfering starts from writer, so there are start values of sems */

    union semun arg;
    arg.val = 2;
    
    if (semctl(semId, SEM_W, SETVAL, arg) == -1)
        ERR_HANDLER("init SEM_W to 2");

    struct sembuf undoW = {SEM_W, -1, SEM_UNDO};

    if (semop(semId, &undoW, 1) == -1)
        ERR_HANDLER("undo SEM_W (-1)");


    /* end of init, to make it clear to the writer that the reader has finished initialization */

        struct sembuf EndInitReader[1] = {
        //{SEM_R_INIT, 0, 0},
        {SEM_R_INIT, +1, SEM_UNDO}
    };

    if (semop(semId, EndInitReader, 1) == -1)
        ERR_HANDLER("The end of critical section of initialization of reader\n"); 

/* end of initialization */

    /* allocate a System V shared memory segment and attach it */

    errno = 0; 

    if ( (shmId = shmget(SHM_KEY, sizeof(struct ShmSeg), IPC_CREAT | OBJ_PERMS)) == -1)
        ERR_HANDLER("shmget");

    if ( (shmSeg = shmat(shmId, NULL, 0)) == (void *) -1)
        ERR_HANDLER("shmat");

    /* wait for initialization from another side */

    struct sembuf reserveInitW[3] = {
        {SEM_W_INIT, -1, 0},
        {SEM_W_INIT, +1, 0},
        {SEM_E, +1, SEM_UNDO}
    };

    if (semop(semId, reserveInitW, 3) == -1)
        ERR_HANDLER("reserve SEM_W_INIT");

    if (DEBUG_REGIME)
        printSem(semId, "after reserve W_INIT\n"
                        "Before while");
 
/* data transfering */

    struct sembuf reserveR = {SEM_R, -1, 0};
    struct sembuf releaseW = {SEM_W, +1, 0};

    errno = 0;

    while (1) 
    {
        if (semop(semId, &reserveR, 1) == -1)
            ERR_HANDLER("reserve READ sem");
        
        /* if another side has already died */

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
        
        if (semop(semId, &releaseW, 1) == -1)
            ERR_HANDLER("release WRITE sem");
    }

    /* give turn to writer to use [sem/shm]ctl and detach shm*/

    struct sembuf endStuff[4] = {
        {SEM_W,       +1, SEM_UNDO},            /* give turn (sign) to writer that we've finished
                                                   and he can RM shm, sem, etc.*/
        {SEM_R_INIT,  -1, SEM_UNDO},
        {SEM_R_ALIVE, -1, SEM_UNDO},
        {SEM_E,       -1, SEM_UNDO},
    };

    if (semop(semId, endStuff, 4) == -1)
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

    fprintf(stderr, "Status of transmission: %s\n", transferErr ? "FAILED" : "SUCCESS");

    exit(EXIT_SUCCESS);
}

