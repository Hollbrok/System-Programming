#include "../Common/info.h"
#include "../Common/debug.h"

void serverInt(int noPc);

int main(int argc, char *argv[])
{
    if ((argc > 1 && strcmp(argv[1], "--help") == 0) || argc != 2)
    {
        fprintf(stderr, "USAGE:\n"
            "1) For one-time launch: %s <NO PCs>\n"
            "2) For loop:            %s --loop\n", argv[0], argv[0]);
        
        exit(EXIT_SUCCESS);
    }
    else if (argc > 1 && strcmp(argv[1], "--loop") == 0)
    {
        while (1)
        {
            printf("Enter number of wanted PCs: ");
            fflush(stdout);

            char noPcStr[MAX_PC_DIGITS + 1];
            fgets(noPcStr, MAX_PC_DIGITS, stdin);
            noPcStr[strlen(noPcStr) - 1] = '\0';
            
            int errorState = 0;
            int noPc = getNumber(noPcStr, &errorState);
            if (noPc > 0 && errorState == 0)
                serverInt(noPc);
            else
                printf("Incorrect NO PCs\n");
        }
    }

    /* 1 argument: NO PCs; and without loop */
    int errorState = 0;
    int noPc = getNumber(argv[1], &errorState);
    if (noPc > 0 && errorState == 0)
        serverInt(noPc);
    else
        printf("Incorrect NO PCs\n");

    DEBPRINT("END OF PROGRAMM\n");
    exit(EXIT_SUCCESS);
}

void serverInt(int noPc)
{
    int					listenFd;
    int                 nonZero;
	struct sockaddr_in  servAddr;

    listenFd = Socket(AF_INET, SOCK_STREAM, 0); /* NON_BLOCK? */

    nonZero = 1;
    Setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &nonZero, sizeof(nonZero));

    struct timeval timeout = {
            .tv_sec = TIMEOUT_SEC,
            .tv_usec = TIMEOUT_USEC
    };

    Setsockopt(listenFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    Setsockopt(listenFd, SOL_SOCKET, SO_KEEPALIVE, &nonZero, sizeof(nonZero));

    // add alive stuff

    bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family      = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port        = htons(SERV_PORT);
    

	Bind(listenFd, (SA *) &servAddr, sizeof(servAddr));

    fprintf(stderr, "SERVER: BIND TCP SOCKET\n");

    DEBPRINT("after BIND\n");

    /* broadcast stuff */

    DEBPRINT("BC START\n");

    struct sockaddr_in bcAddr;
    int bcFd;

    bcFd = Socket(AF_INET, SOCK_DGRAM, 0);
    
    Setsockopt(bcFd, SOL_SOCKET, SO_BROADCAST, &nonZero, sizeof(nonZero));
    Setsockopt(bcFd, SOL_SOCKET, SO_REUSEADDR, &nonZero, sizeof(nonZero));

    bzero(&bcAddr, sizeof(bcAddr));

    bcAddr.sin_family = AF_INET;
    bcAddr.sin_port = htons(CL_PORT);
    bcAddr.sin_addr.s_addr = INADDR_BROADCAST;

    Bind(bcFd, (struct sockaddr *) &bcAddr, sizeof(bcAddr));

    if (DEB_REGIME)
    {
        struct sockaddr ss;
        socklen_t len = sizeof(ss);
        if (getsockname(bcFd, (SA *) &ss, &len) < 0)
            return;
        
        DEBPRINT("server port = %d\n"
                 "server addr = %s\n", htons(((struct sockaddr_in*)(&ss))->sin_port),
                                       inet_ntoa(((struct sockaddr_in*)(&ss))->sin_addr));
    }
    

    /* broadcast */
    
    int bcMsg = SERV_PORT;
    Sendto(bcFd, &bcMsg, sizeof(bcMsg), 0, (struct sockaddr *) &bcAddr, sizeof(bcAddr));        
    close(bcFd);

    DEBPRINT("BC END\n");


    /*  end of broadcast  |
        start of listening  */

    DEBPRINT("before listen bind\n");

	Listen(listenFd, LISTENQ);

    DEBPRINT("after listen\n");

    double totalIntResult = 0;

    int connFds[noPc];

    struct sockaddr_in servAddrs[noPc];
    socklen_t addrLen = sizeof(servAddrs[0]);


    /* waiting for all clients */

    int iClient = 0;

    for (; iClient < noPc; ++iClient)
    {
        DEBPRINT("before accept\n");

	    connFds[iClient] = Accept(listenFd, NULL, NULL);
        
        if (connFds[iClient] == -2) /* EAGAIN == Resource temporarily unavailable */
        {
            DEBPRINT("got all clients\nafter accept\n");

            servAddrs[iClient].sin_family = AF_INET;
            servAddrs[iClient].sin_port   = htons(CL_PORT/*SERV_PORT*/);

            break;
        }

        DEBPRINT("after success accept\n");

        servAddrs[iClient].sin_family = AF_INET;
        servAddrs[iClient].sin_port   = htons(CL_PORT/*SERV_PORT*/);
    }

    DEBPRINT("GOT %d real workers\n", iClient);

    int connPcs = iClient;
    if (connPcs == 0)
    {
        fprintf(stderr, "got 0 workers, exit.\n");
        return;//exit(EXIT_FAILURE);
    }

    /* from now time can be counted */

    struct timeval beginT, endT;
    gettimeofday(&beginT, 0);

    DEBPRINT("start of sending info to clients\n");

    /* got clients NOthreads to optimal divition of the work */

    size_t totalThreads = 0;
    size_t clisThreads[connPcs]; /* no threads of clients */
    for (int iClient = 0; iClient < connPcs; ++iClient)
    {
        struct CliInfo iClientInfo;

        Recv(connFds[iClient], &iClientInfo, sizeof(iClientInfo), 0);

        clisThreads[iClient] = iClientInfo.noThreads;
        totalThreads += iClientInfo.noThreads;
    }

    DEBPRINT("TOTAL Threads = %d\n", totalThreads);

    /* send needed info for calc */

    double lastEnd = GENERAL_START_INT;
    for (int iClient = 0; iClient < connPcs/*noPc*/; ++iClient)
    {
        DEBPRINT("iClient = %d\n", iClient);

        double intLength = (double)(GENERAL_FINISH_INT - GENERAL_START_INT) * clisThreads[iClient]
                        / totalThreads;

        double a = lastEnd;
        double b = a + intLength;
        lastEnd = b;

        struct CalcInfo calcInfo = {
            .a = a,
            .b = b,
        };

        Sendto(connFds[iClient], &calcInfo, sizeof(calcInfo), 0,
                              (struct sockaddr *)(&servAddrs[iClient]), addrLen);
    }

    DEBPRINT("after send data\n");

    /* get results */

    for (int iClient = 0; iClient < connPcs/*noPc*/; iClient++)
    {
        DEBPRINT("getting data from %d client\n", iClient);
        DEBPRINT("CLIENT addr = %s\n", inet_ntoa(servAddrs[iClient].sin_addr));
       
        struct IntResult intRes;
        Recv(connFds[iClient], &intRes, sizeof(intRes), 0);
    
        totalIntResult += intRes.result;
        DEBPRINT("after recv with result = [%lf]\n", intRes.result);


	    close(connFds[iClient]);
    }

    DEBPRINT("after got results\n");


    gettimeofday(&endT, 0);
    long seconds = endT.tv_sec - beginT.tv_sec;
    long microseconds = endT.tv_usec - beginT.tv_usec;
    double elapsed = seconds + microseconds*1e-6;

	fprintf(stderr, "result = %lf\n"
                    "real time of calculation: %.3f secs.\n", totalIntResult, elapsed);

    close(listenFd);
}
