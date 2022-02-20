#include "../headers/libs.h"
#include "../headers/bintree.h"

 
long getNumber(char *numString);

int testForeach(struct bintreeElem *elem, void* x)
{
    if (elem->data_ > *((int *) x))
        printf("[%d]\n", elem->data_);

    return 0;
}

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

    fprintf(stderr, "size of tree = %d.\n", tree->size_);
    fprintf(stderr, "root value = %d\n", tree->root_->data_);

    show_tree(tree);

    clear(tree);
    
    free(tree);


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