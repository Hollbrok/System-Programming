#ifndef SHM_COMMON_H
#define SHM_COMMON_H

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

#define PRINT_INT(number)           \
    do                              \
    {                               \
    fprintf(stderr, #number);       \
    fprintf(stderr, " = %ld",       \
            (long) number);         \
    } while (0);





#endif