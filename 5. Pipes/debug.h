#ifndef DEBUG_H_INC
#define DEBUG_H_INC


#ifdef DEB

#define DEBUG_REGIME 1

#else

#define DEBUG_REGIME 0

#endif

#define NEED_SLEEP   0
#define NEED_LINE    0
#define NEED_PID     1


#define DEBPRINT(args...)                       \
    do                                          \
    {                                           \
        if(DEBUG_REGIME)                        \
        {                                       \
            fprintf(stderr, "\n(%ld): ", (long) getpid() - (long) getppid() - 1);\
            fprintf(stderr, args);              \
        }                                       \
    } while(0)

#define ERRCHECK_CLOSE(FD)      \
    if (close(FD) != 0)         \
    {                           \
        perror("Close #FD");    \
    }    

#define DEB_SLEEP(x, msg)                               \
    if(NEED_SLEEP)                                      \
    {                                                   \
        fprintf(stderr, msg);                           \
        fprintf(stderr, "\n\nbefore test sleep = %d\n", x);     \
        sleep(x);                                               \
        fprintf(stderr, "after test sleep = %d\n\n", x);        \
    }

#define ERR_HANDLER(msg)    \
    do { perror(msg); exit(EXIT_FAILURE); } while(0)


#endif