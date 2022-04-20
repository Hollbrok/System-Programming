#include "../Common/info.h"
#include "../Common/debug.h"

#include "threads_int.h"

void clientInt();

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        err_quit("USAGE: %s\n", argv[0]);
    else if (argc != 1)
        err_quit("Incorrect NO arguments\n"
                 "USAGE: %s\n", argv[0]);

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

    fprintf(stderr, "read info:\n"
                    "iClient = %d\n"
                    "noPc = %d\n"
                    "noThreads = %d\n", calcInfo.iClient, calcInfo.noPc, calcInfo.noThreads);

    double intLength = (float)(GENERAL_FINISH_INT - GENERAL_START_INT)
                        / calcInfo.noPc;

    double a = GENERAL_START_INT + intLength * calcInfo.iClient;
    double b = a + intLength;

        /* add dump if needed */

    fprintf(stderr, "a = %lf, b = %lf\n", a, b);

    struct IntResult intRes;
    intRes.result = calcInt(calcInfo.noThreads, a, b);

    fprintf(stderr, "integtal result = [%lf]", intRes.result);

    /* send to the server result of integral */

    Writen(sockfd, &intRes, sizeof(intRes));

    close(sockfd);

    fprintf(stderr, "SUCCESS\n");
    exit(EXIT_SUCCESS);
}
