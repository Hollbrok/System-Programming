#include "../headers/bintree.h"

int printElem(struct bintreeElem *elem, void *x);

void testForeach(struct bintree *tree);


////////

int main()
{
    struct bintree *tree = (struct bintree *) (calloc(1, sizeof(struct bintree)));
    if (tree == NULL)
    {
        fprintf(stderr, "can't calloc memory for tree.\n");
        exit(ERR_CALLOC);
    }

    createTree(tree);

    add(tree, 5);

    fprintf(stderr, "size  = %d\n"
                    "&root = %p\n", tree->size_, tree->root_);

    testForeach(tree);

    //show_tree(tree);

    clear(tree);

    exit(EXIT_SUCCESS);
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
        if (foreach(OR_T_INORDER, tree, printElem, (void *) -1) != ERR_SUCCESS)
        {
            fprintf(stderr, "[%d].FOREACH test FAILED.\n", i);
            return;
        }
    
    fprintf(stderr, "FOREACH test SUCCESS.\n");
    return;
}