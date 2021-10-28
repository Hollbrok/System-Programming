/* WRITER */
#include "libs.h"
#include "common.h"
#include "debug.h"

/*
TODO: 
1)  Possibility to 1st start reader (not the writer),
    so need to check if sem set or shm already exist.
2)  


*/

int main(int argc, char* argv[])
{
    /* data section */

    int semId, shmId;           /* IPS stuff*/
    int fileRd, lastByteRead;   /* file data transfering stuff*/

    struct ShmSeg * shmSeg;
    union semun uselessArg;

    /* checks if there is a 2nd argument (file name) */

    if (argc < 2)
    {
        DEBPRINT("No file arguments provided\n")
        exit(EXIT_FAILURE);
    }

    /* get (create) a System V semaphore set identifier and initialize them*/

    semId = semGet(WRITER);

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
    

    if( (fileRd = open(argv[1], O_RDONLY)) == -1)
    {
        LEAVE_STUFF
        ERR_HANDLER("open file source")
    }

    /* so sending data to reader */

    printf("before reserver R_INIT\n");

    if (reserveSem(semId, SEM_R_INIT) == -1)
    {
        LEAVE_STUFF
        ERR_HANDLER("reserve SEM_R_INIT")
    }

    printf("after reserve R_INIT\n");

    if (DEBUG_REGIME)
        printSem(semId);

    while (1)
    {     
        /* Wait for our turn */
        //printf("before reserver W_SEM\n");

        if (DEBUG_REGIME)
            printSem(semId);

        if (reserveSem(semId, SEM_W) == -1)
        {
            if (errno == EAGAIN)
            {
                LEAVE_STUFF
                ERR_HANDLER("EAGAIN")
            }
            else
            {
                LEAVE_STUFF
                ERR_HANDLER("reserve WRITE sem (errno !=EINTR)")
            }
        } /* now both SEM_W/R are inUse (value is 0)*/

        //printf("after reserver W_SEM\n");

        if (getSemVal(semId, SEM_E) == 1)
        {
            if (DEBUG_REGIME)
                printSem(semId);

            LEAVE_STUFF
            DEBPRINT("READER DEAD!!!!!!!!!!!\n")
            
            exit(EXIT_FAILURE);
        }

        if ( (lastByteRead = read(fileRd, shmSeg->buf, BUF_SIZE)) == -1)
        {
            LEAVE_STUFF
            ERR_HANDLER("read from file")
        }


        shmSeg->cnt = lastByteRead;
        DEBPRINT("lastByte to shm = %d\n", shmSeg->cnt)

        /* shmSeg is ready for reader so we give reader a turn */
 
        if (releaseSem(semId, SEM_R) == -1)
        {
            LEAVE_STUFF
            ERR_HANDLER("release READ sem")
        }

        if (lastByteRead == 0) /* got EOF*/
        {
            DEBPRINT("find EOF\n")
            break;
        }

        DEBPRINT("done 1 write cicle\n")
    }

    /*  After exiting the loop we must wait till reader will done, then
        detach the shared memory segment and releases the writer semaphore,
        so that the writer program can remove the IPC objects.
    */

    if (reserveSem(semId, SEM_W) == -1)
    {
        LEAVE_STUFF
        ERR_HANDLER("reserver WRITE sem")
    }
    
    LEAVE_STUFF

    /*
    if (semctl(semId, 0, IPC_RMID, uselessArg) == -1)
    {
        perror("remove semId");
        exit(EXIT_FAILURE);
    }
    
    if (shmdt(shmSeg) == -1)
    {
        perror("detach shm");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmId, IPC_RMID, 0) == -1)
    {
        perror("remove shm Seg");
        exit(EXIT_FAILURE);
    }*/

    DEBPRINT("SUCCESS\n");

    exit(EXIT_SUCCESS);
}

