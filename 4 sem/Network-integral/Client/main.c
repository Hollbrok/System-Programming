#include "../Common/info.h"
#include "../Common/debug.h"

#include "threads_int.h"

void clientInt(int noThreads);

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        err_quit("USAGE: %s <NO threads>\n", argv[0]);
    else if (argc != 2)
        err_quit("Incorrect NO arguments\n"
                 "USAGE: %s <NO threads>\n", argv[0]);

    //DEBPRINT("pid = %ld\n", (long)getpid());

    int noThreads = getNumber(argv[1]);
    clientInt(noThreads);

    return EXIT_SUCCESS;
}

void clientInt(int noThreads)
{
    int					sockfd;
	struct sockaddr_in	servaddr;

    /* find broadcast server connection */

    struct sockaddr_in peer_addr;
    struct sockaddr_in sock_addr;

    bzero(&peer_addr, sizeof peer_addr);
    bzero(&sock_addr, sizeof sock_addr);

    socklen_t peer_addr_len = sizeof peer_addr;

    sock_addr.sin_family      = AF_INET;
    sock_addr.sin_port        = htons(CL_PORT);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    
    int sk = Socket(AF_INET, SOCK_DGRAM, 0);

    int nonZero = 1; /* setsockopt requires a nonzero *optval to turn the option on */

    if (setsockopt (sk, SOL_SOCKET, SO_REUSEADDR, &nonZero, sizeof(nonZero)) != 0) 
        err_sys("setsockopt for server listen socket (SO_REUSEADDR)");

    Bind(sk, (struct sockaddr *) &sock_addr, sizeof(sock_addr));

    int server_tcp_port = 0;

    DEBPRINT("go to recv\n");

    while (server_tcp_port != SERV_PORT)
        recvfrom(sk, &server_tcp_port, sizeof (int), 0,
                 (struct sockaddr *) &peer_addr, &peer_addr_len);

    //printf("SERVER TCP PORT = %d\n", server_tcp_port);
    //printf("SERVER ADDR = %s\n", inet_ntoa(peer_addr.sin_addr));

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(SERV_PORT);
    servaddr.sin_addr   = peer_addr.sin_addr;

    socklen_t servAddrLen = sizeof(servaddr);

    DEBPRINT("SERV addr = %s\n", inet_ntoa(peer_addr.sin_addr));

    /* connect to server */

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    //  set alive

    DEBPRINT("before connect to server\n");
	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
    DEBPRINT("after connected to server\n");

    /* main part */

    /* send info about out NO threads*/

    struct CliInfo cliNOThreads = { .noThreads = noThreads};
    Send(sockfd, &cliNOThreads, sizeof(cliNOThreads), 0/*MSG_NOSIGNAL*/);

    /* send 1st msg that we are in */

        //struct ReadyMsg readyMsg = { .r = '1' };

        // Writen(sk/*sockfd*/, &readyMsg, sizeof(readyMsg));
        //int sendRet = Sendto(sockfd, &readyMsg, sizeof(readyMsg), 0,
        //                          (struct sockaddr *)(&servaddr), servAddrLen);

        //fprintf(stderr, "1st send: %d bytes\n", sendRet);
    
    /* receive info about noPc and noThreads */

    struct CalcInfo calcInfo;
    bzero(&calcInfo, sizeof(calcInfo));
    

    Recv(sockfd, &calcInfo, sizeof(calcInfo), 0);
    
    DEBPRINT("after recv calc info\n");

    /* calc integral */

    /*DEBPRINT("read info:\n"
             "iClient = %d\n"
             "noPc = %d\n"
             "totalThreads = %d\n", calcInfo.iClient,
              calcInfo.noPc, calcInfo.noThreads);

    double intLength = (float)(GENERAL_FINISH_INT - GENERAL_START_INT) * noThreads
                        / (calcInfo.totalThreads);

    double a = GENERAL_START_INT + intLength * calcInfo.iClient;
    double b = a + intLength;*/

        /* add dump if needed */

    DEBPRINT("a = %lf, b = %lf\n", calcInfo.a, calcInfo.b);

    struct IntResult intRes;
    intRes.result = calcInt(noThreads/*calcInfo.noThreads*/, calcInfo.a, calcInfo.b);

    DEBPRINT("integtal result = [%lf]\n", intRes.result);

    /* send to the server result of integral */

    Send(sockfd, &intRes, sizeof(intRes), 0/*MSG_NOSIGNAL*/);

    close(sockfd);

    fprintf(stderr, "SUCCESS\n");
    exit(EXIT_SUCCESS);
}
