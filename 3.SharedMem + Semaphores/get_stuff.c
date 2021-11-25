#include "libs.h"
#include "common.h"
#include "debug.h"

void printSem(int semId, const char* msg);

/* creating or getting exclusively shm  + at */

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
