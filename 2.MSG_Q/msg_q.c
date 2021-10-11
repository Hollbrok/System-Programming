#include "libs.h"

static const int MAX_MSG_LENGTH = 1024;

struct mbuf
{
    long mtype;
    char mtext[MAX_MSG_LENGTH];
};

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

long getNumber(char *numString);

int main(int argc, char* argv[])
{
    int NOchild;
    int msgId;
    struct mbuf sendMsg;
    struct mbuf recMsg;


    if (argc != 2)
    {
        fprintf(stderr, "Needs NO childs\n");
        exit(EXIT_FAILURE);
    }

    NOchild = getNumber(argv[1]);

    msgId = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR | IPC_CREAT);
    if (msgId == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "[%d]\n", msgId);
    DEBPRINT("msgQ ID = %d\n", msgId)


    for (int i = 0; i < NOchild; i++)
    {
        switch (fork())
        {
        case -1:    /* Error*/
            perror("fork()");
            exit(EXIT_FAILURE);
            break;
        case 0:     /* Child */
            errno = 0;
            DEBPRINT("before sending\n");
            
            sendMsg.mtype = i + 1;
            snprintf(sendMsg.mtext, MAX_MSG_LENGTH, "Im Child, my pid = %ld\n", (long) getpid());
    
            DEBPRINT("[%s]\nstrlen = %d\n", sendMsg.mtext, strlen(sendMsg.mtext))

            if ( msgsnd(msgId, &sendMsg, strlen(sendMsg.mtext), 0) == -1)
            {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
        
            _exit(EXIT_SUCCESS);
            break;
        default:    /* Parent */
            if (wait(NULL) == -1)
            {
                perror("wait");
                exit(EXIT_FAILURE);
            } 
            DEBPRINT("After wait\n");
            int length = msgrcv(msgId, &recMsg, MAX_MSG_LENGTH, 0, 0);

            DEBPRINT("length = %d\n", length)

            fprintf(stderr, "Got: [%.*s]\n"
                            "type = %ld\n", length, recMsg.mtext, recMsg.mtype);
            break;
        }
    }


    if (msgctl(msgId, IPC_RMID, NULL) != 0)
    {
        fprintf(stderr, "Can't remove msg Queue [%d]\n", msgId);
    }

    DEBPRINT("SUCCESS\n");
    exit(EXIT_SUCCESS);
}

long getNumber(char *numString)
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
        // exit(EXIT_FAILURE);
    }
    if (errno != 0)
    {
        fprintf(stderr, "strtol error\n");
        // exit(EXIT_FAILURE);
    }
    
    return gNumber;

}
