/* WRITER */


#include "libs.h"
#include "common.h"
#include "debug.h"

int main(int argc, char* argv[])
{
    /* data section */
 
    int semId, shmId;           /* IPS stuff                    */
    int fileRd, lastByteRead;   /* file data transfering stuff  */

    struct ShmSeg * shmSeg;
    union semun uselessArg;

    /* checks if there is a 2nd argument (file name) */

    if (argc < 2)
    {
        DEBPRINT("No file arguments provided\n")
        exit(EXIT_FAILURE);
    }

    /* get (create) a System V semaphore set identifier and initialize them*/

    if ( (semId = semget(SEM_KEY, NO_SEMS,  IPC_CREAT | OBJ_PERMS)) == -1) /* if already exists or EXCL creation*/
        ERR_HANDLER("semget");

    DEBPRINT("before init\n")

/* Start of critical section (initialization) */

    struct sembuf StartInitWriter[2] = {
        {SEM_W_INIT, 0, 0},
        {SEM_W_INIT, +1, SEM_UNDO}
    };

    if (semop(semId, StartInitWriter, 2) == -1)
        ERR_HANDLER("Start critical section of initialization of writer\n");    
    

    /* the writer waits for the rest of writers to finish their work */

    struct sembuf checkAnotherWriters[2] = {
        {SEM_W_ALIVE, 0, 0},
        {SEM_W_ALIVE, +1, SEM_UNDO}
    };

    if (semop(semId, checkAnotherWriters, 2) == -1)
        ERR_HANDLER("Start critical section of initialization of writer\n");

    /* after writer die reader won't stay in block */

    struct sembuf undoReleaseR[2] = {
        {SEM_R, 1, 0},
        {SEM_R, -1, SEM_UNDO}
    };

    if (semop(semId, undoReleaseR, 2) == -1)
        ERR_HANDLER("undo release of reader");

    /* cycle of file tranfering starts from writer */

    struct sembuf releaseW = {SEM_W, 1, SEM_UNDO};

    if (semop(semId, &releaseW, 1) == -1)
        ERR_HANDLER("release writer sem");

    /* give turn to another processes for init */
    /*
    struct sembuf endInit = {SEM_W_INIT, -1, SEM_UNDO};

    if (semop(semId, &endInit, 1) == -1)
        ERR_HANDLER("end of initialization of writer");*/
    
/* end of initialization */

    DEBPRINT("after initialization of all sems\n")

    /* allocate a System V shared memory segment and attach it */

    errno = 0;

    if ((shmId = shmget(SHM_KEY, sizeof(struct ShmSeg), IPC_CREAT | OBJ_PERMS)) == -1)
        ERR_HANDLER("shmget");

    if ((shmSeg = shmat(shmId, NULL, 0)) == (void *) -1)
        ERR_HANDLER("shmat");
    
    DEBPRINT("successful shmat\n")  

    if ((fileRd = open(argv[1], O_RDONLY)) == -1)
        ERR_HANDLER("open file source");

    /* so sending data to reader */

    DEBPRINT("before reserver R_INIT\n")

    struct sembuf waitInitR = {SEM_R_INIT, -1, 0};

    if (semop(semId, &waitInitR, 1) == -1)
        ERR_HANDLER("reserve SEM_R_INIT");


    if (DEBUG_REGIME)
        printSem(semId, "after reserve R_INIT\n"
                        "Before while");

    while (1)
    {     
        /* Wait for our turn */

        if (reserveSem(semId, SEM_W) == -1)
        {
            if (errno == EAGAIN)
                ERR_HANDLER("EAGAIN");
            else
                ERR_HANDLER("reserve WRITE sem (errno != EINTR)");
        } /* now both SEM_W/R are inUse (value is 0) */


        /* if another side died */

        if (getSemVal(semId, SEM_R_ALIVE) == 0)
        {
            if (DEBUG_REGIME)
                printSem(semId, "death of reader");

            exit(EXIT_SUCCESS);
        }

        if ( (lastByteRead = read(fileRd, shmSeg->buf, BUF_SIZE)) == -1)
            ERR_HANDLER("read from file");

        shmSeg->cnt = lastByteRead;
        DEBPRINT("lastByte to shm = %d\n", shmSeg->cnt)

        /* shmSeg is ready for reader so we give reader a turn */
 
        if (releaseSem(semId, SEM_R) == -1)
           ERR_HANDLER("release READ sem");

        if (lastByteRead == 0) /* got EOF*/
        {
            DEBPRINT("find EOF\n")
            break;
        }

        DEBPRINT("done 1 write cicle\n")
    }

    /*  After exiting the loop we must wait till reader will done */

    struct sembuf reserveW = {SEM_W, -1, SEM_UNDO};

    if (semop(semId, &reserveW, 1) == -1)
        ERR_HANDLER("release writer sem");

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

    DEBPRINT("SUCCESS\n");

    fprintf(stderr, "SUCCESS\n");


    exit(EXIT_SUCCESS);
}

