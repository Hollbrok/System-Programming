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
    int					listenfd, connfd;
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


	clilen = sizeof(cliaddr);
	connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);

    Writen(connfd, &noPc, sizeof(int));

	close(connfd);			/* parent closes connected socket */
    close(listenfd);
}
