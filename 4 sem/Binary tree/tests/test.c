#include "../headers/bintree.h"

int printElem(struct bintreeElem *elem, void *x);

void testAdd(struct bintree* tree);

void testForeach(struct bintree *tree);

void testConstrDeconstr();

int main(int argc, char *argv[])
{
    struct bintree tree = {};


    createTree(&tree);

    
    testAdd(&tree);

    testForeach(&tree);

    testConstrDeconstr();

    clear(&tree);
    exit(EXIT_SUCCESS);
}

void testAdd(struct bintree* tree)
{
    add(tree, 5);
    add(tree, 2);
    add(tree, 3);
    add(tree, 6);
    add(tree, 100);
    add(tree, 102);
    add(tree, 0);
    add(tree, -1);
    add(tree, 66);
    add(tree, 7);


    add(NULL, 1);
}

int printElem(struct bintreeElem *elem, void *x)
{
    if (elem->data_ > (int) x)
        printf("[%d]\n", elem->data_);
    return 0;
}

void testForeach(struct bintree *tree) 
{
    fprintf(stderr, "TTT: %p\n", tree->root_);
    for (int i = 0; i < 10; i++)
    {
        foreach(OR_T_INORDER, tree, printElem, (void *) -1);
        foreach(OR_T_PREORDER, tree, printElem, (void *) -1);
        foreach(OR_T_POSTORDER, tree, printElem, (void *) -1);
        foreach(OR_T_DEFAULT, tree, printElem, (void *) -1);
    }

    struct bintree treeNullRoot = {.root_ = NULL}; 

    foreach(OR_T_DEFAULT, NULL, printElem, NULL);
    foreach(OR_T_DEFAULT, &treeNullRoot, printElem, NULL);
    foreach(OR_T_DEFAULT, tree, NULL, NULL);
    
    fprintf(stderr, "FOREACH test SUCCESS.\n");
    return;
}

void testConstrDeconstr()
{
    createElem(NULL, 1);
    deconstrElem(NULL);
    createTree(NULL);
}

