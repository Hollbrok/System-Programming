/* WRITER */


#include "libs.h"
#include "common.h"
#include "debug.h"

int main(int argc, char* argv[])
{
    /* data section */
 
    int semId = -1, shmId = -1; /* IPS IDs                    */
    int fileRd, lastByteRead;   /* file data transfering stuff  */

    struct ShmSeg * shmSeg = NULL;
    union semun uselessArg;
    int transferErr = 0;


    /* checks if there is the 2nd argument (file name) */

    if (argc < 2)
    {
        DEBPRINT("No file arguments provided\n")
        exit(EXIT_FAILURE);
    }

    /* get (create) a System V semaphore set identifier and initialize them*/

    if ( (semId = semget(SEM_KEY, NO_SEMS,  IPC_CREAT | OBJ_PERMS)) == -1) /* if already exists or EXCL creation*/
        ERR_HANDLER("semget");
 
    printSem(semId, "before init W");

/* Start of critical section (initialization) */    

    /* the writer waits for the rest of writers to finish their work */

    struct sembuf checkAnotherWriters[2] = {
        {SEM_W_ALIVE, 0, 0},
        {SEM_W_ALIVE, +1, SEM_UNDO}
    };

    if (semop(semId, checkAnotherWriters, 2) == -1)
        ERR_HANDLER("Start critical section of initialization of writer\n");

    /* wait 0 pairs (0 processes) in executing */

    struct sembuf wait0Proc = {SEM_E, 0, 0};

    if (semop(semId, &wait0Proc, 1) == -1)
        ERR_HANDLER("wait 0 processes");

    /* cycle of file tranfering starts from writer */

    union semun arg;
    arg.val = 1;
    
    if (semctl(semId, SEM_R, SETVAL, arg) == -1)
        ERR_HANDLER("init SEM_R to 1");

    struct sembuf undoR = {SEM_R, -1, SEM_UNDO};

    if (semop(semId, &undoR, 1) == -1)
        ERR_HANDLER("undo SEM_R (-1)");
 
    /* end of init, to make it clear to the reader that the writer has finished  initialization */


// скорее всего нужна только вторая операция, т.к. SEM_W_INIT всегда 0, после того, как мы дождались 0 процессов 
    struct sembuf EndInitWriter[2] = {
        {SEM_W_INIT, 0, 0},
        {SEM_W_INIT, +1, SEM_UNDO}
    };

    if (semop(semId, EndInitWriter, 2) == -1)
        ERR_HANDLER("End critical section of initialization of writer\n");

/* end of initialization */


    /* allocate a System V shared memory segment and attach it */

    errno = 0;

    if ((shmId = shmget(SHM_KEY, sizeof(struct ShmSeg), IPC_CREAT | OBJ_PERMS)) == -1)
        ERR_HANDLER("shmget");

    if ((shmSeg = shmat(shmId, NULL, 0)) == (void *) -1)
        ERR_HANDLER("shmat");
    
    if ((fileRd = open(argv[1], O_RDONLY)) == -1)
        ERR_HANDLER("open file source");

    /* wait for initialization from another side */


    struct sembuf waitInitR[3] = {
        {SEM_R_INIT, -1, 0},
        {SEM_R_INIT, +1, 0},
        {SEM_E, +1, SEM_UNDO}
    };

    if (semop(semId, waitInitR, 3) == -1)
        ERR_HANDLER("waint reader initialization ");

    if (DEBUG_REGIME)
        printSem(semId, "after reserve R_INIT\n"
                        "Before while");

    /* data transfering cycle */

    struct sembuf reserveW = {SEM_W, -1, 0};
    struct sembuf releaseR = {SEM_R, +1, 0};

    errno = 0;

    while (1)
    {     
        if (semop(semId, &reserveW, 1) == -1)
            ERR_HANDLER("reserve WRITE sem");

        /* if another side has already died  */

        if (getSemVal(semId, SEM_R_ALIVE) == 0)
        {
            if (DEBUG_REGIME)
                printSem(semId, "death of reader");
    
            transferErr = 1;
            break;
        }

        if ( (lastByteRead = read(fileRd, shmSeg->buf, BUF_SIZE)) == -1)
            ERR_HANDLER("read from file");

        shmSeg->cnt = lastByteRead;

        /* shmSeg is ready for reader so we give reader a turn */
 
        if (semop(semId, &releaseR, 1) == -1)
            ERR_HANDLER("release READ sem");

        if (lastByteRead == 0) /* got EOF*/
        {
            DEBPRINT("find EOF\n")
            break;
        }
    }
    
    /*  After exiting the loop we must wait till reader will done */

   struct sembuf endStuff[4] = {
        {SEM_W,       -1, 0}, 
        {SEM_W_INIT,  -1, SEM_UNDO},
        {SEM_W_ALIVE, -1, SEM_UNDO},
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

