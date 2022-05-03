#ifndef	__debug_h__
#define	__debug_h__

/* debug and errs handling stuff */

#define DEB_REGIME   0
#define NEED_SLEEP   0
#define NEED_LINE    0
#define NEED_PID     0


#define DEBPRINT(args...)                                               \
    do {                                                                \
        if(DEB_REGIME) {                                              \
            if (NEED_LINE)                                              \
                fprintf(stderr, "LINE: %d\n", __LINE__);                \
            if (NEED_PID)                                               \
                fprintf(stderr, "PID: %ld\n", (long) getpid());         \
            fprintf(stderr, args);                                      \
        }                                                               \
    } while(0)


#define ERR_HANDLER(msg)    \
    do { perror(msg); exit(EXIT_FAILURE); } while(0)

#endif	/* __debug_h__ */
