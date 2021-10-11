/*  controling semaphores for synchronization data-transfering/receiving */

#include "libs.h"
#include "common.h"

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
    /*  sem_num    = semNum;
        sem_op     = -1;
        sem_flg    =  0;  
                            */

    struct sembuf sops = {semNum, -1, 0};

    while (semop(semId, &sops, 1) == -1)
    {
        if (errno != EINTR) /* can't break via interrupt */
            return -1;
        else if (errno == EIDRM)
        {
            DEBPRINT("Another process have deleted the semaphore. Exit\n")
            /* TODO: use semctl here to delete sem */
            exit(EXIT_FAILURE); 
        }
        else
            DEBPRINT("[semop error in reserveSem]: errno == %d\n", errno)
    }

    return 0;
}

int releaseSem(int semId, int semNum)
{
    /*  sem_num    = semNum;
        sem_op     = +1;
        sem_flg    =  0;
                            */

    struct sembuf sops = {semNum, +1, 0};

    /* increasing can't be blocked */

    return semop(semId, &sops, 1); 
}