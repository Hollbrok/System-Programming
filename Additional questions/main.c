#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
 
#include <sysexits.h>
#include <err.h>

int fileEq(int fd1, int fd2);

int main(int argc, const char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "argc != 3\n");
        exit(EXIT_FAILURE);
    }

    int fd1 = open(argv[1], 0);
    int fd2 = open(argv[2], 0);

    if (fd1 == -1 || fd2 == -1)
        err(EX_OSERR, "error in opens");

    printf("FDs are %s\n", fileEq(fd1, fd2) == 1 ? "the same": "different");


    return (EXIT_SUCCESS);
}


int fileEq(int fd1, int fd2)
{
    if (fd1 == fd2)
        return 1;

    struct stat sfd1 = {};
    struct stat sfd2 = {};

    if (fstat(fd1, &sfd1) == -1)
    {
        perror("fstat fd1");
        return -1;
    }

    if (fstat(fd2, &sfd2) == -1)
    {
        perror("fstat fd2");
        return -1;
    }

    if (sfd1.st_ino == sfd2.st_ino && sfd1.st_dev == sfd2.st_dev)
        return 1;
    
    return -1;
}