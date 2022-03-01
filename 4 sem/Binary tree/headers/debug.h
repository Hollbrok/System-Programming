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

#define RET_ERR_TYPE enum ERRORS_TYPE

enum ORDERING_TYPE
{
    OR_T_PREORDER, 
    OR_T_POSTORDER,
    OR_T_INORDER,
    OR_T_DEFAULT, /* for tests */

};

enum ERRORS_TYPE
{
    ERROR = -1,
    ERR_SUCCESS = 1,
    ERR_FAILURE,
    ERR_ARGS,
    ERR_TREE_NULL,
    ERR_TREE_ELEM_NULL,
    ERR_CALLOC,
    ERR_ALREADY_EXISTS,
    ERR_EMPTY_TREE,
    ERR_NO_NEED_ELEM,
    ERR_FUNC_NULL,

};


#define DEBPRINT(args...)                       \
    do{ if(DEBUG_REGIME)                            \
        {                                           \
        if (NEED_LINE)                          \
        {                                       \
            if (NEED_PID)                       \
                fprintf(stderr, "\n|[%ld]   ", (long) getpid());    \
            else                                                    \
                fprintf(stderr, "\n|        ");                     \
            fprintf(stderr, "LINE: %d\n"                            \
                            "->", __LINE__);                        \
        }                                                           \
        fprintf(stderr, args);                                      \
        } } while(0)



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