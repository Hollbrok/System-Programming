/*  controling semaphores for synchronization data-transfering/receiving */

#include "libs.h"
#include "common.h"
#include "debug.h"

/* usingState can be 1(available to use) or 0 (in use)  */
/* if error returns -1                                  */

int initSem(int semId, int semNum, enum INIT_SEM usingState)
{
    union semun arg;
    arg.val = usingState;
    
    return semctl(semId, semNum, SETVAL, arg);
}

int reserveSem(int semId, int semNum)
{
    struct sembuf sops = { 
        .sem_num = semNum,
        .sem_op  = -1,
        .sem_flg = 0     };

    errno = 0;

    while (semop(semId, &sops, 1) == -1)
    {
        if (errno == EIDRM)
        {
            DEBPRINT("Another process have deleted the semaphore.Exit\n")
            exit(EXIT_FAILURE); 
        }
        else if (errno == EINTR) /* can't break via interrupt */
        {
            DEBPRINT("errno == EINTR\n")
            perror("");
            return -1;
        }
        else
        {
            DEBPRINT("[semop error in reserveSem]: errno == %d\n", errno)
            return -1;
        }
    }

    return 0;
}

int releaseSem(int semId, int semNum)
{
    DEBPRINT("RELEASE semNum = %d\n", semNum)

    struct sembuf sops = {
        .sem_num = semNum,
        .sem_op  = 1,
        .sem_flg = 0      };

    /* increasing can't be blocked */

    return semop(semId, &sops, 1); 
}

int undoChange(int semId, int semNum, int value)
{
    DEBPRINT("UNDO: Id ; semNum; value\n"
             "      %d     %d      %d \n", semId, semNum, value)

    struct sembuf sops = {
        .sem_num  = semNum,
        .sem_op   = value,
        .sem_flg  = SEM_UNDO };

    errno = 0;

    while (semop(semId, &sops, 1) == -1)
    {
        if (errno == EIDRM)
        {
            DEBPRINT("Another process have deleted the semaphore.Exit\n")
            exit(EXIT_FAILURE); 
        }
        else if (errno == EINTR) /* can't break via interrupt */
        {
            perror("semop");
            return -1;
        }
        else
        {
            perror("semop");
            return -1;
        }
    }

    return 0;
}