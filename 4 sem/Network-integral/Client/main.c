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

	//char sendline[MAXLINE] = "test1";
    char recvline[MAXLINE];
    //Writen(sockfd, sendline, strlen(sendline));

    int noRead;
    if ((noRead = Readn(sockfd, recvline, MAXLINE)) == 0)//Readline(sockfd, recvline, MAXLINE) == 0)
		err_quit("ReadLine: server terminated prematurely");
    else 
        fprintf(stderr, "recover %d bytes\n", noRead);
    
    int *numberp = (int *)recvline;
    fprintf(stderr, "[%d]", *numberp);
    write(STDOUT_FILENO, recvline, strlen(recvline));

    //str_cli(stdin, sockfd);		/* do it all */

    fprintf(stderr, "SUCCESS\n");
    exit(EXIT_SUCCESS);
}
