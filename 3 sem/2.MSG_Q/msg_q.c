#include "libs.h"
#include "debug.h"

static const int MAX_MSG_LENGTH = 1024;

struct mbuf
{
    long mtype;
    char mtext[MAX_MSG_LENGTH];
};


long getNumber(char *numString);


/*

0. n раз форкаем.
1. Затем все дети кидают в очередь_1 родителю все их порядковые номеры (с 1 по n)
2. Родитель читает все этим номера и кидают в очередь_2 ответ (msgtype = SERIAL NUMBER)
3. дети после того, как в П1 отправили родителю сообщение ждут ответа, а затем распечатывают его. 
    [еще дети как только распечатали свой порядковый номер могут отправить сообщение в очередь_1,
     тогда, как только родитель получит сообщение он отправит сообщение в очередь_2 следующему ребёнку и т.п.]

*/

int main(int argc, char* argv[])
{
    int NOchild;

    int msgId1;
    int msgId2;
    
    struct mbuf sendMsg;
    struct mbuf recMsg;

    /* also can use atexit*/

    if (argc != 2)
    {
        fprintf(stderr, "Needs NO childs\n");
        exit(EXIT_FAILURE);
    }

/* */

    NOchild = getNumber(argv[1]);

    msgId1 = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR | IPC_CREAT);
    if (msgId1 == -1)
    {
        perror("msgget1");
        exit(EXIT_FAILURE);
    }

    DEBPRINT("msgQ1 ID = %d\n", msgId1)

    msgId2 = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR | IPC_CREAT);
    if (msgId2 == -1)
    {
        perror("msgget2");
        exit(EXIT_FAILURE);
    }

    DEBPRINT("msgQ2 ID = %d\n", msgId2)

/*  */

    int isParent = 1;
    int serNumber = -1;

    for (int i = 0; i < NOchild && isParent; i++)
    {
        switch (fork())
        {
        case -1:    /* Error*/
            perror("fork()");
            exit(EXIT_FAILURE);
            break;
        case 0:     /* Child */
            DEBPRINT("child\n");
            
            serNumber = i + 1;
            isParent = 0;
            
            break;
        default:    /* Parent */
            
            DEBPRINT("parent\n")
            isParent = 1;
        }
    }

/* */

    if (isParent)
    {
        for (int i = 0; i < NOchild; i++)
        {
            DEBPRINT("i = %d\n", i)

            sendMsg.mtype = i + 1;
            snprintf(sendMsg.mtext, MAX_MSG_LENGTH, "%d\n", i + 1);

            if (msgsnd(msgId1, &sendMsg, strlen(sendMsg.mtext), 0) == -1)
            {
                perror("PARENT: msgsnd");
                exit(EXIT_FAILURE);
            }

            if (msgrcv(msgId2, &recMsg, MAX_MSG_LENGTH, i + 1, 0) == -1)
            {
                perror("PARENT: msgrcv");
                exit(EXIT_FAILURE);
            }

        }

        /* after get response from all child */
        
        if (msgctl(msgId1, IPC_RMID, NULL) != 0)
        {
            fprintf(stderr, "Can't remove msgQ1 [%d]\n", msgId1);
        }

        if (msgctl(msgId2, IPC_RMID, NULL) != 0)
        {
            fprintf(stderr, "Can't remove msgQ1 [%d]\n", msgId2);
        }

    }
    else /* Child */
    {
        int retval = 0;
        if ( (retval = msgrcv(msgId1, &recMsg, MAX_MSG_LENGTH, serNumber, 0)) == -1)
        {
            DEBPRINT("ret val = %d\n")
            perror("CHILD: msgrcv");
            _exit(EXIT_FAILURE);
        }

        printf("%d ", serNumber);

        sendMsg.mtype = serNumber;
        snprintf(sendMsg.mtext, MAX_MSG_LENGTH, "%d\n", serNumber);

        if (msgsnd(msgId2, &sendMsg, strlen(sendMsg.mtext), 0) == -1)
        {
            perror("CHILD: msgsnd");
            _exit(EXIT_FAILURE);
        }

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
