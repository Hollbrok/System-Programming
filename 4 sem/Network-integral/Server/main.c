#include "../Common/info.h"
#include "../Common/debug.h"

void serverInt(int noPc, int noThreads);

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        err_quit("USAGE: %s  <NO pc> <NO threads>\n", argv[0]);
    else if (argc != 3)
        err_quit("Incorrect NO arguments\n"
                 "USAGE: %s <NO pc> <NO threads>\n", argv[0]);

    int noPc = getNumber(argv[1]);
    int noThreads = getNumber(argv[2]);

    serverInt(noPc, noThreads);

    fprintf(stderr, "SUCCESS\n");
    exit(EXIT_SUCCESS);
}

void serverInt(int noPc, int noThreads)
{
    int					listenfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;


    listenfd = Socket(AF_INET, SOCK_STREAM, 0); /* NON_BLOCK? */

    int nonZero = 1;

    if (setsockopt (listenfd, SOL_SOCKET, SO_REUSEADDR, &nonZero, sizeof(nonZero)) != 0) 
        err_sys("setsockopt for server listen socket (SO_REUSEADDR)");

    struct timeval timeout = 
    {
            .tv_sec = TIMEOUT_SEC,
            .tv_usec = TIMEOUT_USEC
    };

    if (setsockopt (listenfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
        err_sys("setsockopt for server listen socket (SO_RCVTIMEO)");

    if (setsockopt (listenfd, SOL_SOCKET, SO_KEEPALIVE, &nonZero, sizeof(nonZero)) != 0)
        err_sys("setsockopt for server listen socket (SO_KEEPALIVE)");

    // keep alive stuff

    bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY/*INADDR_LOOPBACK*/);
	servaddr.sin_port        = htons(SERV_PORT);
    
    //Inet_pton(AF_INET, "10.55.129.99", &servaddr.sin_addr.s_addr);


	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

    DEBPRINT("after BIND\n");

    /* broadcast stuff */

    DEBPRINT("BC START\n");

    struct sockaddr_in bcAddr;
    int bcfd;

    bcfd = Socket(AF_INET, SOCK_DGRAM, 0);

    //int nonZero = 1; /* setsockopt requires a nonzero *optval to turn the option on */
    if (setsockopt (bcfd, SOL_SOCKET, SO_BROADCAST, &nonZero, sizeof(nonZero)) < 0)
    {
        close(bcfd);
        err_sys("setsockopt (BC)");
    }

    if (setsockopt (bcfd, SOL_SOCKET, SO_REUSEADDR, &nonZero, sizeof(nonZero)) != 0) 
        err_sys("setsockopt for server listen socket (SO_REUSEADDR)");

    bzero(&bcAddr, sizeof(bcAddr));

    bcAddr.sin_family = AF_INET;
    bcAddr.sin_port = htons(CL_PORT);
    bcAddr.sin_addr.s_addr = INADDR_BROADCAST;

    Bind(bcfd, (struct sockaddr *) &bcAddr, sizeof(bcAddr));

    struct sockaddr ss;
    socklen_t len = sizeof(ss);
    if (getsockname(bcfd, (SA *) &ss, &len) < 0)
        return;
        
    DEBPRINT("server port = %d\n"
           "server addr = %s\n", htons(((struct sockaddr_in*)(&ss))->sin_port),
                                 inet_ntoa(((struct sockaddr_in*)(&ss))->sin_addr));


    for (int sendConn = 0; sendConn < 1/*noPc*/; ++sendConn)
    {
        int msg = SERV_PORT;

        DEBPRINT("sendConn = %d\n", sendConn);

        Sendto(bcfd, &msg, sizeof msg, 0, (struct sockaddr *) &bcAddr, sizeof(bcAddr));        
    }

    close(bcfd);

    DEBPRINT("BC END\n");


    /*  end of broadcast  |
        start of listening  */

    DEBPRINT("before listen bind\n");

	Listen(listenfd, LISTENQ);

    DEBPRINT("after listen\n");

    double totalIntResult = 0;

    int connFds[noPc];

    struct sockaddr_in servAddrs[noPc];
    socklen_t addrLen = sizeof(servAddrs[0]);


    /* waiting for all clients */

    int iClient = 0;

    for (; iClient < noPc; iClient++)
    {
        clilen = sizeof(cliaddr);

        DEBPRINT("before accept\n");

	    connFds[iClient] = Accept(listenfd, (SA *) &cliaddr, &clilen);
        
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

        //if (Readn(bcfd/*connFds[iClient]*/, &getReady, sizeof(getReady)) != sizeof(getReady))
        //    err_quit("Readn: server terminated prematurely (read ready msg)");
    }

    DEBPRINT("GOT %d real workers\n", iClient);

    int connPcs = iClient;


    /* from now time can be counted */

    struct timeval beginT, endT;
    gettimeofday(&beginT, 0);

    DEBPRINT("start of sending info to clients\n");

    /* send needed info for calc */
    for (int iClient = 0; iClient < connPcs/*noPc*/; iClient++)
    {
        DEBPRINT("iClient = %d\n", iClient);

        struct CalcInfo calcInfo = {
            .iClient = iClient,
            .noPc = connPcs,        /*NO connected PCs */
            .noThreads = noThreads,
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

    close(bcfd);
}
