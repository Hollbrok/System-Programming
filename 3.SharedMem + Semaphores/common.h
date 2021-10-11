#ifndef SHM_COMMON_H
#define SHM_COMMON_H

enum INIT_SEM
{
    InUse,
    AvailableToUse,
};

/* are used in calls to semctl() */

union semun 
{  
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

/* initialized binary semaphore in 1 of 2 possible states: [In use] or [Available to use] */

int initSem(int semId, int semNum, enum INIT_SEM usingState);


/* reserver and release binaty semaphore (-1 and +1 operations respectively) */ 

int reserveSem(int semId, int semNum);
int releaseSem(int semId, int semNum);


/* well-known keys for getting shm and sem */

#define SHM_KEY 0xDEAD
#define SEM_KEY 0x1000

/* Permissions for our IPC objects */

#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

/* semaphore sequence numbers */

#define SEM_W 0
#define SEM_R 1


/* size of info part (buf) of shmseg*/

#define BUF_SIZE 1024

/* structure of shared mem segment */

struct ShmSeg 
{
    int cnt;            /* real size of data in buf*/
    char buf[BUF_SIZE]; /* data being transferred */
};


#define DEBUG_REGIME 1

#define DEBPRINT(args...)   \
    if(DEBUG_REGIME)        \
        fprintf(stderr, args);

#define ERRCHECK_CLOSE(FD)          \
    do                              \
    {                               \
        if (close(FD) != 0)         \
        {                           \
            fprintf(stderr, #FD);   \
            perror("");             \
        }                           \
    } while(0);

#define PRINT_INT(number)               \
    do                                  \
    {                                   \
        fprintf(stderr, #number);       \
        fprintf(stderr, " = %ld",       \
                (long) number);         \
    } while (0);

#endif