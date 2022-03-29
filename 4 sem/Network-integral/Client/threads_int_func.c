#include "threads_int.h"

/* integral of what function will be calculated */
static double function(double x)
{
    return cos( pow(x, 5) * sin(atan(x)) );  
}

/* stuff to calculate integral */

static void *threadInfoConstr(size_t noThreads, size_t *size)
{
    int lineSize = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);

    *size = (sizeof(ThreadInfo) / lineSize + 1) * lineSize;

    return malloc(noThreads * (*size));
}

static void initThreadsInfo(void *info, size_t sizeThreadInfo, int noProc, 
                            int noThreads, int noEmptyThreads, double intLength)
{
    if (noThreads > noProc)
        intLength = intLength * noThreads / noProc;

    /* for ordered threads */
    for (int iThread = 0; iThread < noThreads /* + noEmptyThreads*/; ++iThread)
    {
        /* for this threads we shouldn't set CPU affinity to not to slow down*/
        if (iThread >= noProc)
        {
            ((ThreadInfo *) (info + iThread * sizeThreadInfo))->numCPU = -2;
            continue;
        } 
        
        ((ThreadInfo *)(info + iThread * sizeThreadInfo))->numCPU = iThread % noProc;

        ((ThreadInfo *) (info + iThread * sizeThreadInfo))->a = START_LIMIT + iThread * intLength;

        ((ThreadInfo *) (info + iThread * sizeThreadInfo))->b = ((ThreadInfo *)(info + iThread * sizeThreadInfo))->a + intLength;
    }

    /* for empty threads if they exist */
    for (int iThread = noThreads; iThread < noThreads + noEmptyThreads; iThread++) 
    {
        ((ThreadInfo*) (info + iThread * sizeThreadInfo))->a = START_LIMIT;
        
        ((ThreadInfo*) (info + iThread * sizeThreadInfo))->b = START_LIMIT + intLength;
        
        ((ThreadInfo *) (info + iThread * sizeThreadInfo))->numCPU = -1;
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

    /* noThreads > noProc in system => returns null for no slowdown*/
    if (numCPU == -2)
        return NULL;

    /* calculate int */
    for (double x = threadsInfo->a; x < threadsInfo->b; x += DELTA_X)
        threadsInfo->sum += function(x) * DELTA_X;

    return NULL;
}

static long getNumber(char *numString)
{
    if (*numString == '\0')
    {
        fprintf(stderr, "empty number argument\n");
        exit(EXIT_FAILURE);
    }

    long gNumber;
    char* endOfEnter;

    const int baseOfNumber = 10;
    errno = 0;

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

static void dumpThreadsInfo(void *info, size_t sizeThreadInfo, int noThreads, 
                            int noEmptyThreads)
{
    for (int iThread = 0; iThread < noThreads + noEmptyThreads; ++iThread)
    
        fprintf(stderr, "[%d] numCPU = %d\n", iThread, ((ThreadInfo *) (info + iThread * sizeThreadInfo))->numCPU);

    return;
}

/* calculate integral of function from 0 to 1 in <argv1> threads */
double calcInt(char *strNum)
{
    /* usr number of threads to calculate int */
    int noThreads = getNumber(strNum);
    if (noThreads <= 0)
    {
        fprintf(stderr, "NO threads must be > 0\n");
        return NAN;
    }

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


    /* comment will be here */

    pthread_t* threadsID = (pthread_t *)calloc(maxThreads, sizeof(pthread_t));
    if (threadsID == NULL)
        ERR_HANDLER("Can`t calloc memory for threadsID");

    initThreadsInfo(threadsInfo, sizeThreadInfo, noProc, 
                    noThreads, noEmptyThreads, 
                    (FINISH_LIMIT - START_LIMIT) / noThreads);

    //dumpThreadsInfo(threadsInfo, sizeThreadInfo, noThreads, noEmptyThreads);

    /* creating of threads */

    for (int iThread = 0; iThread < maxThreads; iThread++) 
    {
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
            finalSum += ((ThreadInfo *)(threadsInfo + iThread * sizeThreadInfo))->sum;
    }

    /* printing result and exit s*/

    free(threadsInfo);
    free(threadsID);

    return finalSum;
}

////



