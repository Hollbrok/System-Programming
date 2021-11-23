#include "libs.h"
#include "common.h"
#include "debug.h"

void printSem(int semId, const char* msg);


/* creating or getting exclusively sem set */
/* TYPES of USERs: 
    WRITER = 0;
    READER = 1; */

int semGet(enum TYPE typeOfUser)
{
    int semId;
    if ( (semId = semget(SEM_KEY, NO_SEMS,  IPC_CREAT | IPC_EXCL | OBJ_PERMS)) == -1) /* if already exists or EXCL creation*/
    {
        DEBPRINT("can't create EXCL sem\n")   

        if (errno != EEXIST)        /* Unexpected error from semget() */
            ERR_HANDLER("semget");

        if ( (semId = semget(SEM_KEY, 0, 0)) == -1)
            ERR_HANDLER("semget");

        DEBPRINT("semId = %d\n", semId)
                
        if (DEBUG_REGIME)
            printSem(semId, "before initialization (already exists sem)\n");

        
        /* NEED to wait while who created to initialize and if another writer or reader is dead */

        if (undoChange(semId, typeOfUser == WRITER ? BECOME_W : BECOME_R, -1) == -1)
            ERR_HANDLER("UNDO reserver BECOME");
        if (reserveSem(semId, typeOfUser == WRITER ? SEM_R_INIT : SEM_W_INIT) == -1)
            ERR_HANDLER("reserve another INIT");
        if (releaseSem(semId, typeOfUser == WRITER ? SEM_R_INIT : SEM_W_INIT) == -1)
            ERR_HANDLER("releave another INIT");

        /* restarting semset in incorrect exit at the last time */

        if (getSemVal(semId, EXCL_ALIVE) == 0) /* EXCL is dead */
        {
            fprintf(stderr, "EXCL_ALIVE == 0\n");

            if (semctl(semId, 0, IPC_RMID, NULL) == -1)                               
            {
                if(errno != EINVAL)                             
                    ERR_HANDLER("remove semId"); 
                
                fprintf(stderr, "EINVAL: incorrect semId\n");
                exit(EXIT_FAILURE); 
            }
                   
            /* if last exit was incorrect, so there can be situation with incorrect shm
               and we should remove shm */

            int shmId = shmget(SHM_KEY, 0, 0);

            if (shmctl(shmId, IPC_RMID, 0) == -1)              
                ERR_HANDLER("remove shm Seg");    

            return semGet(typeOfUser);
        }

        /* release == +1*/

        if (releaseSem(semId, typeOfUser == WRITER ? SEM_R : SEM_W) == -1)
            ERR_HANDLER("release WRITE sem");

        if (undoChange(semId, typeOfUser == WRITER ? SEM_R : SEM_W, -1) == -1)
            ERR_HANDLER("UNDO");

        /*  */

        if (undoChange(semId, SEM_E, 1) == -1)
            ERR_HANDLER("UNDO E");

        /* done initialization */
        if (releaseSem(semId, typeOfUser == WRITER ? SEM_W_INIT : SEM_R_INIT) == -1)
            ERR_HANDLER("release WRITE sem");
    }
    else
    {
        DEBPRINT("sem created EXCL [id = %d]\n Initialization\n", semId)
        
        if (undoChange(semId, EXCL_ALIVE, 1) == -1) /*  who created should delete, so if EXCL_ALIVE == 0,
                                                        and are in unexcl creation semset so we need to "restart" semset */
            ERR_HANDLER("UNDO EXCL_ALIVE");

        if (DEBUG_REGIME)
            printSem(semId, "EXCL should be == 1\n");

        if (initSem(semId, SEM_W, 1) == -1)
            ERR_HANDLER("initSem  W(1)");
        
        if (initSem(semId, BECOME_W, 1) == -1)
            ERR_HANDLER("initSem HAS_W(1)");
        
        if (initSem(semId, BECOME_R, 1) == -1)
            ERR_HANDLER("initSem HAS_R(1)");

        if (undoChange(semId, typeOfUser == WRITER ? BECOME_W : BECOME_R, -1) == -1)
            ERR_HANDLER("undo reserve HAS");

        if (releaseSem(semId, typeOfUser == WRITER ? SEM_R : SEM_W) == -1)
            ERR_HANDLER("release WRITE sem");

        if (undoChange(semId, typeOfUser == WRITER ? SEM_R : SEM_W, -1) == -1)
            ERR_HANDLER("UNDO");

        if (undoChange(semId, SEM_E, 1) == -1)
            ERR_HANDLER("UNDO");
        
        /* pass initialization, so we need to release INIT sem to give turn to another process */

        if (releaseSem(semId, typeOfUser == WRITER ? SEM_W_INIT : SEM_R_INIT) == -1)
            ERR_HANDLER("release INIT sem");
    }

    DEBPRINT("AFTER INIT:\n")  
    if (DEBUG_REGIME)
        printSem(semId, "final values");

    return semId;
}

/* creating or getting exclusively shm  + at */

int shmGet()
{
    int shmId;

    if ( (shmId = shmget(SHM_KEY, sizeof(struct ShmSeg), IPC_CREAT | IPC_EXCL | OBJ_PERMS)) == -1)
    {
        DEBPRINT("can't create EXCL shm\n")

        if (errno != EEXIST) /* Unexpected error from semget() */
            ERR_HANDLER("shmget");

        shmId = shmget(SHM_KEY, 0, 0);
        
        DEBPRINT("shmId = %d\n", shmId)

        if (shmId == -1)
            ERR_HANDLER("shmget");
    }
    else
        DEBPRINT("shm created EXCL\n")

    return shmId;
}

/* print info about sem values */

void printSem(int semId, const char* msg)
{
    fprintf(stderr, "%s\n", msg);

    struct semid_ds ds;
    union semun arg;

    arg.buf = &ds;  

    if (semctl(semId, 0, IPC_STAT, arg) == -1)
        ERR_HANDLER("semctl");

    arg.array = calloc(ds.sem_nsems, sizeof(arg.array[0]));

    if (arg.array == NULL)
        ERR_HANDLER("calloc");
    if (semctl(semId, 0, GETALL, arg) == -1)
        ERR_HANDLER("semctl-GETALL");

    char semName[NO_SEMS][20] = { SEM_NAMES };

    fprintf(stderr, "Sem # Value    NAME\n");

    for (int j = 0; j < ds.sem_nsems; j++)
        fprintf(stderr, "%3d %5d %s\n", j, arg.array[j], semName[j]);

    free(arg.array);
}

int getSemVal(int semId, int semNum)
{
    struct semid_ds ds;
    union semun arg;

    arg.buf = &ds;  

    if (semctl(semId, 0, IPC_STAT, arg) == -1)
        ERR_HANDLER("semctl");

    arg.array = calloc(ds.sem_nsems, sizeof(arg.array[0]));

    if (arg.array == NULL)
        ERR_HANDLER("calloc");
    if (semctl(semId, 0, GETALL, arg) == -1)
        ERR_HANDLER("semctl-GETALL");

    //fprintf(stderr, "semNum = %d\n", semNum);

    int retval = arg.array[semNum];
    free(arg.array);

    return retval;
}
