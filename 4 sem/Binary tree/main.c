#include "headers/libs.h"
#include "headers/bintree.h"

 
long getNumber(char *numString);

int testForeach(struct bintreeElem *elem, void* x)
{
    if (elem->data_ > (int) x)
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
    /*add(tree, 4);
    add(tree, 2);
    add(tree, 3);
    add(tree, 6);
    add(tree, 100);
    add(tree, 102);
    add(tree, 0);
    add(tree, -1);
    add(tree, 66);
    add(tree, 7);
    add(tree, 1); */


    fprintf(stderr, "size of tree = %d.\n", tree->size_);
    fprintf(stderr, "root value = %d\n", tree->root_->data_);

    //foreach(OR_T_INORDER, tree, testForeach, (void *) -1);

    //clear(tree);
    //add(tree, 1);

    //printf("9 %s here.\n", (search(tree, 9) == -1 ? "isn't" : "is" ));
    //printf("5 %s here.\n", (search(tree, 5) == -1 ? "isn't" : "is" ));

    show_tree(tree);

    clear(tree);

    fprintf(stderr, "after clear\n");

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