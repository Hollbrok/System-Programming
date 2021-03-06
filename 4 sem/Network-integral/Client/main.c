#include "../Common/info.h"
#include "../Common/debug.h"

#include "threads_int.h"

void clientInt(int noThreads);

int main(int argc, char *argv[])
{
    if ((argc > 1 && strcmp(argv[1], "--help") == 0) || argc != 2)
        err_quit("USAGE: \n"
                 "for specific performance : %s <NO threads>\n"
                 "for max performance      : %s -1\n", argv[0], argv[0]);

    //DEBPRINT("pid = %ld\n", (long)getpid());

    int errorState = 0;
    int noThreads = getNumber(argv[1], &errorState);

    if (noThreads > 0 && errorState == 0)
        while (1)
            clientInt(noThreads);
    if (noThreads == -1) /* using max performance */
        while (1)
            clientInt(sysconf(_SC_NPROCESSORS_ONLN));
    else
        printf("Incorrect NO Threads\n");

    return EXIT_SUCCESS;
}

void clientInt(int noThreads)
{
    int                 sockFd;
    struct sockaddr_in  servAddr;

    /* find broadcast server connection */

    struct sockaddr_in peerAddr;
    struct sockaddr_in sockAddr;

    bzero(&peerAddr, sizeof peerAddr);
    bzero(&sockAddr, sizeof sockAddr);

    socklen_t peerAddrLen = sizeof peerAddr;

    sockAddr.sin_family      = AF_INET;
    sockAddr.sin_port        = htons(CL_PORT);
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    
    int bcsk = Socket(AF_INET, SOCK_DGRAM, 0);

    int nonZero = 1; /* setsockopt requires a nonzero *optval to turn the option on */

    Setsockopt(bcsk, SOL_SOCKET, SO_REUSEADDR, &nonZero, sizeof(nonZero));

    Bind(bcsk, (struct sockaddr *) &sockAddr, sizeof(sockAddr));

    int servTcpPort = 0;

    DEBPRINT("go to recv\n");

    fprintf(stderr, "Waiting for server..\n");
    recvfrom(bcsk, &servTcpPort, sizeof (int), 
                 0, (struct sockaddr *) &peerAddr, &peerAddrLen);
    fprintf(stderr, "Got server.\n");
    if (servTcpPort != SERV_PORT)
    {
        fprintf(stderr, "error on recv server port occurs\n");
        return;
    }

    close(bcsk);

    //printf("SERVER TCP PORT = %d\n", servTcpPort);
    //printf("SERVER ADDR = %s\n", inet_ntoa(peerAddr.sin_addr));

    bzero(&servAddr, sizeof(servAddr));

    servAddr.sin_family = AF_INET;
    servAddr.sin_port   = htons(SERV_PORT);
    servAddr.sin_addr   = peerAddr.sin_addr;

    socklen_t servAddrLen = sizeof(servAddr);

    DEBPRINT("SERV addr = %s\n", inet_ntoa(peerAddr.sin_addr));

    /* connect to server */

    sockFd = Socket(AF_INET, SOCK_STREAM, 0);

    /* broken connection stuff */
    int keepCnt     = KEEP_CNT;
    int keepIdle    = KEEP_IDLE;
    int keepIntvl   = KEEP_INTVL;

    Setsockopt(sockFd, IPPROTO_TCP, TCP_KEEPCNT, &keepCnt, sizeof(int));
    Setsockopt(sockFd, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
    Setsockopt(sockFd, IPPROTO_TCP, TCP_KEEPINTVL, &keepIntvl, sizeof(int));

    /* */

    DEBPRINT("Client: connecting TCP socket\n");

    Connect(sockFd, (SA *) &servAddr, sizeof(servAddr));
    
    DEBPRINT("after connected to server\n");

        /* main part */

    /* send info about out NO threads*/

    struct CliInfo cliNOThreads = { .noThreads = noThreads};
    Send(sockFd, &cliNOThreads, sizeof(cliNOThreads), MSG_NOSIGNAL);
    
    /* receive info about noPc and noThreads */

    struct CalcInfo calcInfo;
    bzero(&calcInfo, sizeof(calcInfo));
    

    Recv(sockFd, &calcInfo, sizeof(calcInfo), 0);
    
    DEBPRINT("after recv calc info\n");

    /* calc integral */

    DEBPRINT("a = %lf, b = %lf\n", calcInfo.a, calcInfo.b);

    struct IntResult intRes;
    intRes.result = calcInt(noThreads, calcInfo.a, calcInfo.b);

    DEBPRINT("integtal result = [%lf]\n", intRes.result);

    /* send to the server result of integral */

    Send(sockFd, &intRes, sizeof(intRes), MSG_NOSIGNAL);

    close(sockFd);

    fprintf(stderr, "SUCCESS\n");
    return;
}
