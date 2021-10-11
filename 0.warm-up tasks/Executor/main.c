#include "libs.h"

int main(int argc, char *argv[], char *env[])
{
    if(argc < 2)
    {
        fprintf(stderr, "INVALID NO arguments\n");
        exit(EXIT_FAILURE);
    }
    
    /*
    int i = 1;

    while(argv[i])
    {
        printf("[%s]\n", argv[i++]);
    }
    printf("///////////\n");
    */
    
    switch (fork())
    {
    case -1:
        perror("fork()");
        break;
    case 0:
        execv(argv[1], &argv[1]);
        perror("EXIT error\n");
        break;
    default:
        fprintf(stdout, "MAIN: SUCCESS\n");
        break;
    }
    
    

    exit(EXIT_SUCCESS);
}
