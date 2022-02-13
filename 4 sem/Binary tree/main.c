#include "libs.h"
#include "bintree.h"

 
long getNumber(char *numString);

int main(int argc, char *argv[])
{
    struct bintree *tree = (struct bintree *) (calloc(1, sizeof(struct bintree)));
    if (tree == NULL)
    {
        fprintf(stderr, "can't calloc memory for tree.\n");
        exit(ERR_CALLOC);
    }

    createTree(tree);

    add(tree, 5);
    add(tree, 4);
    add(tree, 2);
    add(tree, 3);
    add(tree, 6);
    add(tree, 100);
    add(tree, 0);
    add(tree, -1);
    add(tree, 5);
    add(tree, 66);
    add(tree, 7);
    add(tree, 1);

    fprintf(stderr, "size of tree = %d.\n", tree->size_);

    show_tree(tree);

    exit(EXIT_SUCCESS);
}

long getNumber(char *numString)
{
    if (numString == NULL)
    {
        fprintf(stderr, "null string argument\n");
    }
    if (*numString == '\0')
    {
        fprintf(stderr, "empty number argument\n");
        exit(ERR_ARGS);
    }

    errno = 0;

    long gNumber;
    char* endOfEnter;

    const int baseOfNumber = 10;
    gNumber = strtol(numString, &endOfEnter, baseOfNumber);

    if(*endOfEnter != '\0')
    {
        fprintf(stderr, "strtol error\n");
        exit(ERR_ARGS);
    }
    if (errno != 0)
    {
        fprintf(stderr, "strtol error\n");
        exit(ERR_ARGS);
    }
    
    return gNumber;

}