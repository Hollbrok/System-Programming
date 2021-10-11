/* READER */


#include "libs.h"
#include "common.h"

int main(int argc, char* argv[])
{
    /* data section */
    
    int semId, shmId;
    int lastByteRead;

    struct ShmSeg *shmSeq;

    /* get already exists sem's and shm + attach shm */

    if ( (semId = semget(SEM_KEY, 0, 0)) == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    
    if ( (shmId = shmget(SHM_KEY, 0, 0)) == -1)
    {
        perror("shmget");
        EXIT(EXIT_FAILURE);
    }
    
    if ( (shmSeq = shmat(shmId, NULL, SHM_RDONLY)) == (void *) -1)
    {
        perror("shmat (RDONLY)");
        EXIT(EXIT_FAILURE);
    }

    /* get info in a loop */

    while (1)
    {
        if (reserveSem(semId, SEM_R) == -1)
        {
            perror("reserver Read sem");
            exit(EXIT_FAILURE);
        } 

        if (shmSeq->cnt == 0)
            break;

        fprintf(stderr, "%*.s", shmSeq->cnt, shmSeq->buf);


        if (releaseSem(semId, SEM_W) == -1)
        {
            perror("release Write sem");
            exit(EXIT_FAILURE);
        }
    }

    if (shmdt(shmSeq) == -1)
    {
        perror("detach shm");
        exit(EXIT_FAILURE);
    }

    /* give turn to writer to use [sem/shm]ctl  and detach shm*/
    if (releaseSem(semId, SEM_W) == -1)
    {
        perror("release Write sem");
        exit(EXIT_FAILURE);
    }


    exit(EXIT_SUCCESS);
}

