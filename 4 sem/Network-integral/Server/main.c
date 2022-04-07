#include "../Common/unp.h"
#include "../Common/debug.h"

void serverInt(int noPc, int noThreads);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "ERROR. use: %s <NO pc> <NO threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    DEBPRINT("pid = %ld\n", (long)getpid());

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

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

    double totalIntResult = 0;

    int connFds[noPc];

    /* waiting for all clients */
    for (int iClient = 0; iClient < noPc; iClient++)
    {
        clilen = sizeof(cliaddr);
	    connFds[iClient] = Accept(listenfd, (SA *) &cliaddr, &clilen);

        struct ReadyMsg getReady;
        if (Readn(connFds[iClient], &getReady, sizeof(getReady)) != sizeof(getReady))
            err_quit("Readn: server terminated prematurely (read ready msg)");
    }

    /* from now time can be counted */
    struct timeval beginT, endT;
    gettimeofday(&beginT, 0);

    fprintf(stderr, "starting..\n");

    /* send needed info for calc */
    for (int iClient = 0; iClient < noPc; iClient++)
    {
        struct CalcInfo calcInfo = {
            .iClient = iClient,
            .noPc = noPc,
            .noThreads = noThreads,
        };
        Writen(connFds[iClient], &calcInfo, sizeof(calcInfo));
    }

    /* get results */
    for (int iClient = 0; iClient < noPc; iClient++)
    {
        struct IntResult intRes;
        if (Readn(connFds[iClient], &intRes, sizeof(intRes)) != sizeof(intRes))
            err_quit("Readn: server terminated prematurely (read int result)");

        totalIntResult += intRes.result;

	    close(connFds[iClient]);
    }

    gettimeofday(&endT, 0);
    long seconds = endT.tv_sec - beginT.tv_sec;
    long microseconds = endT.tv_usec - beginT.tv_usec;
    double elapsed = seconds + microseconds*1e-6;

	fprintf(stderr, "result = %lg\n"
                    "time: %.3f secs.\n", totalIntResult, elapsed);
    close(listenfd);
}
