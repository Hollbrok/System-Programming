#include "../headers/bintree.h"

int printElem(struct bintreeElem *elem, void *x);

int main(int argc, char *argv[])
{
    struct bintree *tree_2 = createTree();

    //initTree(&tree_2);

    clear(tree_2);

    struct bintree tree; 
    struct bintree *ptree = NULL;

    if ( (ptree = createTree()) == NULL) /* first call of MY_CALLOC returns NULL*/
        ptree = createTree();

    testAdd(ptree);

    testForeach(ptree);

    testRemoveElem_and_Show(ptree);

    testSearch(ptree);

    testConstrDeconstr();

    clear(ptree);
    
    free(ptree);
    free(tree_2);


    //struct bintreeElem *testElem;
    //fprintf(stderr, "addr = %p\n", testElem);

    exit(EXIT_SUCCESS);
}

void testAdd(struct bintree* tree)
{
    fprintf(stderr, "root = %p\n", tree->root_);
    add(tree, 5);
    add(tree, 5);
    add(tree, 2);
    add(tree, 3);
    add(tree, 16);
    add(tree, 15);
    add(tree, 14);
    add(tree, 100);
    add(tree, 102);
    add(tree, 0);
    add(tree, -1);
    add(tree, 66);
    add(tree, 17);

    //addTo(NULL, NULL);

    add(NULL, 1);

    fprintf(stderr, "ADD test - SUCCESS.\n");
}

int printElem(struct bintreeElem *elem, void *x)
{
    if (elem->data_ > (int) x)
        printf("[%d]\n", elem->data_);
    return 0;
}

void testForeach(struct bintree *tree) 
{
    for (int i = 0; i < 10; i++)
    {
        foreach(OR_T_INORDER, tree, printElem, (void *) -1);
        foreach(OR_T_PREORDER, tree, printElem, (void *) -1);
        foreach(OR_T_POSTORDER, tree, printElem, (void *) -1);
        foreach(OR_T_DEFAULT, tree, printElem, (void *) -1);
    }

    struct bintree *treeNullRoot = createTree(); 

    foreach(OR_T_DEFAULT, NULL, printElem, NULL);
    foreach(OR_T_DEFAULT, treeNullRoot, printElem, NULL);
    foreach(OR_T_DEFAULT, tree, NULL, NULL);

    //foreachFrom(OR_T_DEFAULT, NULL, printElem, NULL);
    
    fprintf(stderr, "FOREACH test - SUCCESS.\n");

    clear(treeNullRoot);
    free(treeNullRoot);

    return;
}

void testRemoveElem_and_Show(struct bintree *tree)
{
    removeElem(NULL, 1);

    struct bintree treeNullRoot = {.root_ = NULL}; 
    removeElem (&treeNullRoot, 1);
    clear(&treeNullRoot);

    show_tree(tree);

    removeElem(tree, 5);
    removeElem(tree, 66);
    removeElem(tree, -1);
    removeElem(tree, 6);

    //removeElemFrom(NULL, 1);

    struct bintree testTree = {.root_ = NULL}; 

    /* tests of operations on the left subtree */

    add(&testTree, 10);
    add(&testTree, 9);

    removeElem(&testTree, 10);
    removeElem(&testTree, 9);

    
    add(&testTree, 10);
    add(&testTree, 11);

    removeElem(&testTree, 11);

    add(&testTree, 9);
    removeElem(&testTree, 9);

    removeElem(&testTree, 10);

    add(&testTree, 100);
    add(&testTree, 19);
    add(&testTree, 20);

    removeElem(&testTree, 19);

    add(&testTree, 29);
    add(&testTree, 28);
    add(&testTree, 27);

    removeElem(&testTree, 20);
    removeElem(&testTree, 28);

    removeElem(&testTree, 29);

    /* on the right subtree*/

    clear(&testTree);
    //clearFrom(NULL);


    add(&testTree, 11);
    add(&testTree, 10);

    removeElem(&testTree, 10);
    removeElem(&testTree, 11);

    add(&testTree, 100);
    add(&testTree, 119);
    add(&testTree, 120);

    removeElem(&testTree, 119);

    
    add(&testTree, 129);
    add(&testTree, 128);
    add(&testTree, 127);

    removeElem(&testTree, 120);
    removeElem(&testTree, 128);

    removeElem(&testTree, 129);

    clear(&testTree);

    clear(NULL); 
    
    /* to test removeFrom needs "big" tree */

    add (tree, 120);
    add (tree, 129);
    add (tree, 122);
    add (tree, 123);
    add (tree, 124);
    add (tree, 125);
    add (tree, 132);
    add (tree, 135);
    add (tree, 137);
    add (tree, 134);
    add (tree, 119);
    add (tree, 125);
    add (tree, 126);
    add (tree, 121);
    add (tree, 127);
    add (tree, 128);


    add (tree, -120);
    add (tree, -121);
    add (tree, -122);
    add (tree, -123);
    add (tree, -124);
    add (tree, -125);
    add (tree, -110);
    add (tree, -111);
    add (tree, -112);
    add (tree, -113);


    removeElem(tree, 124);
    removeElem(tree, 125);
    removeElem(tree, -124);
    removeElem(tree, -122);

    removeElem(tree, -120);
    removeElem(tree, -125);


    add(tree, 136);
    removeElem(tree, 135);

    removeElem(tree, 137);
    removeElem(tree, 128);

    show_tree(&testTree);
    


    show_tree(NULL);


    struct bintree nullRoot = {.root_ = NULL };
    clear(&nullRoot);
    show_tree(&nullRoot);

    clear(&testTree);

    fprintf(stderr, "REMOVE and SHOW tests - SUCCESS.\n");


    return;
}

void testSearch(struct bintree *tree)
{
    search(NULL, 1);
    //searchFrom(NULL, 1);

    struct bintree nullRoot = {.root_ = NULL };
    search(&nullRoot, 1);

    add(tree, 10);
    add(tree, 14);
    add(tree, 12);
    add(tree, 22);
    add(tree, 26);
    add(tree, 24);
    add(tree, 6);
    add(tree, 2);
    add(tree, -2);
    add(tree, 8);
    add(tree, 4);
    add(tree, 0);

    search(tree, 24);

    clear(&nullRoot);

    fprintf(stderr, "SEARCH test - SUCCESS.\n");

}

void testConstrDeconstr()
{
    deconstrElem(NULL);

    fprintf(stderr, "(DE)/CONSTR test - SUCCESS.\n");
}

