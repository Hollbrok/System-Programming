#include "../Common/unp.h"
#include "../Common/debug.h"

#include "threads_int.h"

void clientInt();

int main(int argc, char *argv[])
{
    if (argc != 1)
    {
        fprintf(stderr, "ERROR. use: %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    DEBPRINT("pid = %ld\n", (long)getpid());

    clientInt();

    return EXIT_SUCCESS;
}

void clientInt()
{
    int					sockfd;
	struct sockaddr_in	servaddr;

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

    /* main part */

    /* send 1st msg that we are in */

    struct ReadyMsg readyMsg = { .r = '1' };
    Writen(sockfd, &readyMsg, sizeof(readyMsg));

    /* recover info about noPc and noThreads */

    struct CalcInfo calcInfo;
    bzero(&calcInfo, sizeof(calcInfo));

    int noRead;
    if ((noRead = Readn(sockfd, &calcInfo, sizeof(calcInfo))) == 0)
		err_quit("Readn: client terminated prematurely");
    else 
        fprintf(stderr, "recover %d bytes\n", noRead);

    /* calc integral */

    double intLength = (GENERAL_FINISH_INT - GENERAL_START_INT) 
                        / calcInfo.noPc;

    double a = GENERAL_START_INT + intLength * calcInfo.iClient;
    double b = a + intLength;

        /* add dump if needed */

    struct IntResult intRes;
    intRes.result = calcInt(calcInfo.noThreads, a, b);

    /* send to the server result of integral */

    Writen(sockfd, &intRes, sizeof(intRes));

    close(sockfd);

    fprintf(stderr, "SUCCESS\n");
    exit(EXIT_SUCCESS);
}
