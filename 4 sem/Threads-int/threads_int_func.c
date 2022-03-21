#include "threads_int.h"

static IntFunc function = NULL;

/* calculate integral of function from 0 to 1 in <argv1> threads */

double calcInt(char *strNum, IntFunc intFunc)
{
    function = intFunc;

    /* usr number of threads to calculate int */
    int noThreads = getNumber(strNum);

    /* get number of processors currently available in the system */
    int noProc = sysconf(_SC_NPROCESSORS_ONLN);

    /*  NOempty is 0 if NO currently available processors in the system is <= usr requesting 
        if usr request of threads is < CA processors => noEmptyThreads > 0                      */
    int noEmptyThreads = noProc > noThreads? noProc - noThreads : 0;

    /* max NO threads of noProc and noThreads */
    int maxThreads = MAX(noProc, noThreads);

    /* allocate memory, but we should take into consideration cache line size */

    size_t sizeThreadInfo = 0;
    void* threadsInfo = threadInfoConstr(maxThreads, &sizeThreadInfo);
    if (threadsInfo == NULL)
        ERR_HANDLER("AllocThreadInfo");


    /*  */

    pthread_t* threadsID = (pthread_t*)calloc(maxThreads, sizeof(pthread_t));
    if (threadsID == NULL)
        ERR_HANDLER("Can`t calloc memory for threadsID");

    initThreadsInfo(threadsInfo, sizeThreadInfo, 
                    noThreads, noEmptyThreads, 
                    (FINISH_LIMIT - START_LIMIT) / noThreads);

    /* creating of threads */

    for (int iThread = 0; iThread < maxThreads; iThread++) 
    {
        if (noEmptyThreads > 0)
            ((ThreadInfo*)(threadsInfo + iThread * sizeThreadInfo))->numCPU = iThread;

        if (pthread_create(threadsID + iThread, NULL, 
                           pthreadStartFunc, (threadsInfo + iThread * sizeThreadInfo)) != 0)
            ERR_HANDLER("Can`t create thread");
    }


    /* waiting all threads to calculate */

    double finalSum = 0;
    
    for (int iThread = 0; iThread < maxThreads; iThread++) 
    {
        if (pthread_join(threadsID[iThread], NULL) != 0)
            ERR_HANDLER("pthread_join non-empty Threads");

        if (iThread < noThreads)
            finalSum += ((ThreadInfo*)(threadsInfo + iThread * sizeThreadInfo))->sum;
    }

    /*for (int iThread = noEmptyThreads; iThread < maxThreads; iThread++) 
    {
        if (pthread_join(threadsID[iThread], NULL) != 0)
            ERR_HANDLER("pthread_join empty Thread");
    }*/

    /* printing result and exit s*/

    //fprintf(stdout,"\tIntegral value - %lg", finalSum);

    free(threadsInfo);
    free(threadsID);

    return finalSum;
}

/* stuff to calculate integral */

static void *threadInfoConstr(size_t noThreads, size_t *size)
{
    int lineSize = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);

    *size = (sizeof(ThreadInfo) / lineSize + 1) * lineSize;

    return malloc(noThreads * (*size));
}

static void initThreadsInfo(void *info, size_t sizeThreadInfo, int noThreads,
                            int noEmptyThreads, double intLength)
{
    /* general initialization stuff */
    for (int iThread = 0; iThread < noThreads + noEmptyThreads; ++iThread)
    {
        ((ThreadInfo*) (info + iThread * sizeThreadInfo))->dX = DELTA_X;
    
        ((ThreadInfo*) (info + iThread * sizeThreadInfo))->numCPU = -1;
    }

    /* for ordered threads */
    for (int iThread = 0; iThread < noThreads; iThread++) 
    {
        ((ThreadInfo*) (info + iThread * sizeThreadInfo))->a = START_LIMIT + iThread * intLength;

        ((ThreadInfo*) (info + iThread * sizeThreadInfo))->b = ((ThreadInfo *)(info + iThread * sizeThreadInfo))->a + intLength;
    }

    /* for empty threads */
    for (int iThread = noThreads; iThread < noThreads + noEmptyThreads; iThread++) 
    {
        ((ThreadInfo*) (info + iThread * sizeThreadInfo))->a = START_LIMIT;
        
        ((ThreadInfo*) (info + iThread * sizeThreadInfo))->b = START_LIMIT + intLength;
    }
}

static void *pthreadStartFunc(void* arg)
{    
    ThreadInfo* threadsInfo = (ThreadInfo*) arg;
    int numCPU              = ((ThreadInfo*) threadsInfo)->numCPU;


    /*  if there is >0 empty threads we should 
        set CPU affinity of a threads except zero */
    if (numCPU > 0)
    {
        cpu_set_t cpu = {};
        pthread_t id  = pthread_self();

        CPU_ZERO(&cpu);
        CPU_SET(numCPU, &cpu);

        if (pthread_setaffinity_np(id, sizeof(cpu_set_t), &cpu) < 0)
            ERR_HANDLER("Error in setaffinity");
    }

    /* calculate int */
    for (double x = threadsInfo->a; x < threadsInfo->b; x += threadsInfo->dX)
        threadsInfo->sum += function(x) * threadsInfo->dX;

    return NULL;
}

static long getNumber(char *numString)
{

    if (*numString == '\0')
    {
        fprintf(stderr, "empty number argument\n");
        exit(EXIT_FAILURE);
    }

    errno = 0;

    long gNumber;
    char* endOfEnter;

    const int baseOfNumber = 10;
    gNumber = strtol(numString, &endOfEnter, baseOfNumber);

    if(*endOfEnter != '\0')
    {
        fprintf(stderr, "strtol error\n");
        exit(EXIT_FAILURE);
    }
    if (errno != 0)
    {
        fprintf(stderr, "strtol error\n");
        exit(EXIT_FAILURE);
    }
    
    return gNumber;
}
