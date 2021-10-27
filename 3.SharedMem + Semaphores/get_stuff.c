#include "libs.h"
#include "common.h"
#include "debug.h"

void printSem(int semId);

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
        

        if (errno != EEXIST) /* Unexpected error from semget() */
        {
            //LEAVE_STUFF
            ERR_HANDLER("semget")
        }

        if ( (semId = semget(SEM_KEY, 0, 0)) == -1)
            ERR_HANDLER("semget");
        
        DEBPRINT("semId = %d\n", semId)

        /* NEED to wait while who created to initialize */

        if (reserveSem(semId, typeOfUser == WRITER ? SEM_R_INIT : SEM_W_INIT) == -1)
            ERR_HANDLER("reserve another INIT")
        if (releaseSem(semId, typeOfUser == WRITER ? SEM_R_INIT : SEM_W_INIT) == -1)
            ERR_HANDLER("releave another INIT")


        DEBPRINT("BEFORE INIT:\n")
        printSem(semId);

        /* release == +1*/


        if (releaseSem(semId, typeOfUser == WRITER ? SEM_R : SEM_W) == -1)
            ERR_HANDLER("release WRITE sem")

        if (undoChange(semId, typeOfUser == WRITER ? SEM_R : SEM_W, -1) == -1)
            ERR_HANDLER("UNDO");

        if (undoChange(semId, SEM_E, 1) == -1)
            ERR_HANDLER("UNDO");

        if (releaseSem(semId, typeOfUser == WRITER ? SEM_W_INIT : SEM_R_INIT) == -1)
            ERR_HANDLER("release WRITE sem")
    }
    else
    {
        DEBPRINT("sem created EXCL [id = %d]\n Initialization\n", semId)
    
        if (initSem(semId, SEM_W, AvailableToUse) == -1)
            ERR_HANDLER("initSem  W(1)")
        
        if (initSem(semId, SEM_R, InUse) == -1)
            ERR_HANDLER("initSem  R(0)")
        
        if (initSem(semId, SEM_E, 0) == -1)
            ERR_HANDLER("initSem  E(0)")

        if (initSem(semId, SEM_W_INIT, 0) == -1)
            ERR_HANDLER("initSem W_INIT(0)")
        
        if (initSem(semId, SEM_R_INIT, 0) == -1)
            ERR_HANDLER("initSem R_INIT(0)")
    
        DEBPRINT("going to UNDO %s\n", typeOfUser == WRITER ? "READ sem" : "WRITE sem")

        if (releaseSem(semId, typeOfUser == WRITER ? SEM_R : SEM_W) == -1)
            ERR_HANDLER("release WRITE sem")

        if (undoChange(semId, typeOfUser == WRITER ? SEM_R : SEM_W, -1) == -1)
            ERR_HANDLER("UNDO");

        DEBPRINT("AFTER UNDO\n")

        if (undoChange(semId, SEM_E, 1) == -1)
            ERR_HANDLER("UNDO");
        
        /* DONE */

        if (releaseSem(semId, typeOfUser == WRITER ? SEM_W_INIT : SEM_R_INIT) == -1)
            ERR_HANDLER("release INIT sem");
    }

    DEBPRINT("AFTER INIT:\n")
    printSem(semId);

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
        {
            //LEAVE_STUFF
            ERR_HANDLER("shmget")
        }

        shmId = shmget(SHM_KEY, 0, 0);
        
        DEBPRINT("shmId = %d\n", shmId)

        if (shmId == -1)
        {
            //LEAVE_STUFF
            ERR_HANDLER("shmget")
        }
    }
    else
    {
        DEBPRINT("shm created EXCL\n")
    }

    return shmId;


}

/* print info about sem values */
void printSem(int semId)
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

    char semName[5][10] = { "WRITER", "READER", "ERROR", "W_READY", "R_READY" };

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

    //char semName[5][10] = { "WRITER", "READER", "ERROR", "W_READY", "R_READY" };

    //fprintf(stderr, "Sem # Value    NAME\n");

    //for (int j = 0; j < ds.sem_nsems; j++)
    //    fprintf(stderr, "%3d %5d %s\n", j, arg.array[j], semName[j]);
    int retval = arg.array[semNum];
    free(arg.array);

    return retval;
}
