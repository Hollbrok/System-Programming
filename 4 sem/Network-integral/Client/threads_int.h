#ifndef THREADS_INT_H
#define THREADS_INT_H

#include "../Common/libs.h"
#include "../Common/debug.h"

/* headers of functions */

typedef double (*IntFunc)(double);

typedef struct ThreadInfo 
{
    //IntFunc func;         /* integral of what function will be calculated                 */

    double sum;             /* value of integral on [ startX ; finishX ]                    */

    double a;               /* value from from which the calculation of the integral begins */
    double b;               /* value of the end of the integral calculation                 */
    
    int numCPU;             /* to set CPU affinity of the thread if there was empty threads */

} ThreadInfo;

/* integral of what function will be calculated */
static double function(double x);

/* main function for integral calculation; returns result of integral sum */
double calcInt(char *strNum);

/* cmd line number parser ( returns <long> number according to numString with error handling) */
static long getNumber(char *numString);

/* allocate memory for threads data according to cache line size */
static void *threadInfoConstr(size_t num_threads, size_t *size);

/* initialize a,b, deltaX, numCPU according to type of threads (main or empty) */
static void initThreadsInfo(void *info, size_t sizeThreadInfo, int noProc,
                            int noThreads, int noEmptyThreads, double intLength);

/* function that every thread will execute */
static void *pthreadStartFunc(void *arg);

static void dumpThreadsInfo(void *info, size_t sizeThreadInfo, int noThreads, int noEmptyThreads);

/* info stuff */

static const double START_LIMIT  = 0;
static const double FINISH_LIMIT = 5;
static const double DELTA_X      = 0.0000001; /* calculation offset (accuracy) */

/*  [all results were obtained using wolframalpha]
    correct value of 
    integral of cos( Power[x,5] * sin(ArcTan[x])) from
    
    0 to 100 is 0.94167317124834
    0 to 10  is 0.941672 
    0 to 1   is 0.979753
*/


#endif