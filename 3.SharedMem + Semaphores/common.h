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

enum TYPE
{
    WRITER,
    READER,
};

int semGet(enum TYPE typeOfUser);
int shmGet();
void printSem(int semId, const char* msg);
int getSemVal(int semId, int semNum);

 
/* well-known keys for getting shm and sem */

#define SHM_KEY 0xDEADDEAD
#define SEM_KEY 0xDEADDEAD

//#define SAVE_KEY 0xFFDEAD

/* Permissions for our IPC objects */

#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

/* semaphore sequence numbers in set*/


#define SEM_W 0 /* binary sem that we use like mutex for critical section */
#define SEM_R 1 /*                          -//-                          */

#define SEM_E 2 /* to find out if 1 of processes are terminated */

#define SEM_W_INIT 3    /* to ensure mutual exclusion on initialization */
#define SEM_R_INIT 4    /*                  -//-                        */

#define BECOME_W 5      /* to ensure there is only one writer */
#define BECOME_R 6      /* to ensure there is only one reader */

#define EXCL_ALIVE 7   /* to remove ipc in case of unexpected termination */

#define NO_SEMS 8

#define SEM_NAMES           \
    "WRITER", "READER",     \
    "ERROR",                \
    "W_READY", "R_READY",   \
    "BECOME_W", "BECOME_R", \
    "EXCL_ALIVE"


/* size of info part (buf) of shmseg*/

#define BUF_SIZE 4096

/* structure of shared mem segment */

struct ShmSeg 
{
    int  cnt;           /* real size of data in buf*/
    char buf[BUF_SIZE]; /* data being transferred */
};

#define LEAVE_STUFF                                     \
        fprintf(stderr, "test\n");
    
    /*if (semctl(semId, 0, IPC_RMID, NULL) == -1)         \
    {                                                   \
        if(errno != EINVAL)                             \
            ERR_HANDLER("remove semId");                \
                                                        \
        fprintf(stderr, "EINVAL: incorrect semId\n");   \
        exit(EXIT_FAILURE);                             \
    }                                                   \
    if (shmdt(shmSeg) == -1)                            \
        ERR_HANDLER("detach shm");                      \
    if (shmctl(shmId, IPC_RMID, 0) == -1)               \
        ERR_HANDLER("remove shm Seg");    */                                           

#endif