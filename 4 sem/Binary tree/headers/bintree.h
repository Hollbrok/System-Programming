#ifndef BINTREE_H
#define BINTREE_h

#include "bintree_elem.h"

/* The binary sort tree (left - less, right - over) */

struct bintree
{
	struct bintreeElem *root_;  // = nullptr;

    int size_;                  // = 0;
};

/////

void *TEST_calloc(size_t nmemb, size_t size);

RET_ERR_TYPE initTree(struct bintree* newTree);

struct bintree* createTree();

/* */

RET_ERR_TYPE add(struct bintree* tree, int value);

/* */

RET_ERR_TYPE removeElem(struct bintree* tree, int value);

/*  */

RET_ERR_TYPE clear(struct bintree* tree);

/* -1 if there is no elem with data == value in tree, another value on success. */

RET_ERR_TYPE search(struct bintree* tree, int value);

/* Postorder foreach */

RET_ERR_TYPE foreach(enum ORDERING_TYPE orderType, struct bintree *tree, int (func)(struct bintreeElem *, void *), void *x);

//// DUMP

void show_tree(struct bintree* tree);


#endif