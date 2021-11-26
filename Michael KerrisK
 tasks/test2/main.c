#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <sysexits.h>
#include <err.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

void handler(int sig){
    printf("1");
}

int main()
{
    signal(SIGSEGV, handler);
    printf("before\n");
    * (int *) 6 = 5;
    printf("ok\n");
    return 0;
}