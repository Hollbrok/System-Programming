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
    
    execv(argv[1], &argv[1]);
    perror("EXIT error\n");

    exit(EXIT_SUCCESS);
}
