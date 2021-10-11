/* WRITER */
#include "libs.h"
#include "common.h"


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

    /* checks if there is a 2nd argument (file name) */

    if (argc != 2)
    {
        DEBPRINT("No file arguments provided\n")
        exit(EXIT_FAILURE);
    }

    /* get (create) a System V semaphore set identifier and initialize them*/

    
    if ( (semId = semget(SEM_KEY, 2, IPC_CREAT | OBJ_PERMS)) == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    if (initSem(semId, SEM_W, AvailableToUse) == -1)
    {
        perror("initSem (AVB)");
        exit(EXIT_FAILURE);
    }
    if (initSem(semId, SEM_R, InUse) == -1)
    {
        perror("initSem (InUse)");
        exit(EXIT_FAILURE);
    }

    /* allocate a System V shared memory segment and attach it */

    if ( (shmId = shmget(SHM_KEY, sizeof(struct ShmSeg), IPC_CREAT | OBJ_PERMS)) == -1)
    {
        perror("shm attach");
        exit(EXIT_FAILURE);
    }

    if ( (shmSeg = shmat(shmId, NULL, 0)) == (void *) -1)
    {
        perror("shm attach");
        exit(EXIT_FAILURE);
    }

    /* so sending data to reader */

    if( (fileRd = open(argv[1], O_RDONLY)) == -1)
    {
        perror("open file source");

        /* TODO: use semctl here to delete sem and shm*/
        exit(EXIT_FAILURE);
    }

    while (1)
    {     
        /* Wait for our turn */
        if (reserveSem(semId, SEM_W) == -1)
        {
            perror("reserver Write Sem");
            exit(EXIT_FAILURE);
        } /* now both SEM_W/R are used (value is 0)*/

        if ( (lastByteRead = read(fileRd, shmSeg->buf, BUF_SIZE)) == -1)
        {
            perror("read from file");
            exit(EXIT_FAILURE);
        }


        shmSeg->cnt = lastByteRead;
        DEBPRINT("lastByte to shm = %d\n", shmSeg->cnt)

        /* shmSeg is ready for reader so we give reader a turn */

        if (releaseSem(semId, SEM_R) == -1)
        {
            perror("release Read sem");
            exit(EXIT_FAILURE);
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

    union semun uselessArg;

    if (reserveSem(semId, SEM_W) == -1)
    {
        perror("reserve Write Sem");
        exit(EXIT_FAILURE);
    }
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
    }

    DEBPRINT("SUCCESS\n");

    exit(EXIT_SUCCESS);
}

