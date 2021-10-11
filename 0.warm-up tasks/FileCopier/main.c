#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

const int MAX_BUFFER_SIZE = 4096;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Please, enter two file names\n");
        return 0; // exit(EXIT_FAILURE);
    }
    
    if (argc > 3)
    {
        fprintf(stderr, "To many arguments");
        return 0; // exit(EXIT_FAILURE);
    }

    const char *fileName1, *fileName2;

    fileName1 = argv[1];
    fileName2 = argv[2];
    
    // TASK:
    // I need to read X bytes of info from file1 to BUFFER, then write them into file2. So I need to do this in cycle while I can.

    char buffer[MAX_BUFFER_SIZE]; // this is my temp buffer

    errno = 0;

    int fileDiscr1;
    fileDiscr1 = open(fileName1, O_RDONLY);

    if (fileDiscr1 == -1)  
    {
        fprintf(stderr, "bad open 1 file\n");
        return 0; // exit(EXIT_FAILURE);
    }

    if (errno != 0)  
    {
        fprintf(stderr, "possible errors: ENOENT, EACCES, EEXIST\n");
        return 0; // exit(EXIT_FAILURE);
    }

    int fileDiscr2;
    fileDiscr2 = open(fileName2, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fileDiscr2 == -1)  
    {
        fprintf(stderr, "bad open 2 file\n");
        return 0; // exit(EXIT_FAILURE);
    }

    if (errno != 0)  
    {
        fprintf(stderr, "possible errors: ENOENT, EACCES, EEXIST\n");
        return 0; // exit(EXIT_FAILURE);
    }

    size_t lastByteRead = read(fileDiscr1, (char*) buffer, MAX_BUFFER_SIZE);
    while ( lastByteRead != 0)
    {
        int writeRes = write(fileDiscr2, (char*) buffer, lastByteRead);
        
        if (writeRes != lastByteRead)
        {
            fprintf(stderr, "Error write\n");
            return 0; // exit(EXIT_FAILURE)
        }
        // Should I add write-error handler?

        lastByteRead = read(fileDiscr1, (char*) buffer, MAX_BUFFER_SIZE);
    }

    close(fileDiscr1);
    close(fileDiscr2);

    return 1;
}