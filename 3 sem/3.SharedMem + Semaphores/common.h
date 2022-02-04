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
int undoChange(int semId, int semNum, int value);

/*  */

enum USER_TYPE
{
    WRITER,
    READER,
};

int semGet(enum USER_TYPE typeOfUser);

int shmGet();

int getSemVal(int semId, int semNum);

void printSem(int semId, const char* msg);

 
/* well-known keys for getting shm and sem */

#define SHM_KEY 0xDEADDEAD
#define SEM_KEY 0xDEADDEAD

/* Permissions for our IPC objects */

#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

/* semaphore sequence numbers in set*/

 
#define SEM_W 0 /* binary sem that we use like mutex for critical section */
#define SEM_R 1 /*                          -//-                          */

#define SEM_E 2 /* to find out if 1 of processes are terminated (equal to Number of processes) */

#define SEM_W_INIT 3    /* to ensure mutual exclusion on initialization */
#define SEM_R_INIT 4    /*                  -//-                        */

#define SEM_W_ALIVE 5   /* to ensure there is only one writer */
#define SEM_R_ALIVE 6   /* to ensure there is only one reader */



#define NO_SEMS 7

#define SEM_NAMES           \
    "WRITER", "READER",     \
    "ERROR",                \
    "W_INIT", "R_INIT",   \
    "W_ALIVE", "R_ALIVE", \

/* size of info part (buf) of shmseg*/

#define BUF_SIZE 4096

/* structure of shared mem segment */

struct ShmSeg 
{
    int  cnt;           /* real size of data in buf*/
    char buf[BUF_SIZE]; /* data being transferred */
};
                                       

#endif