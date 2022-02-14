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

/*

add
contains
remove
clear

dump

foreach {1st argument is the type of ordering: PreOrder, PostOrder, InOrder}

*/

RET_ERR_TYPE createTree(struct bintree* newTree);

RET_ERR_TYPE add(struct bintree* tree, int value);

RET_ERR_TYPE addTo(struct bintreeElem* mainElem, struct bintreeElem* insertElem);

RET_ERR_TYPE removeElem(struct bintree* tree, int value);

RET_ERR_TYPE removeElemFrom(struct bintreeElem* mainElem, int value);

RET_ERR_TYPE clear(struct bintree* tree);

RET_ERR_TYPE clearFrom(struct bintreeElem* mainElem);

/* Postorder foreach */

RET_ERR_TYPE foreach(enum ORDERING_TYPE orderType, struct bintree *tree, int (func)(struct bintreeElem *, void *), void *x);

RET_ERR_TYPE foreachFrom(enum ORDERING_TYPE orderType, struct bintreeElem *mainElem, int (func)(struct bintreeElem *, void *), void *x);


//// DUMP

void show_tree(struct bintree* tree);

void graphviz_beauty_dump(struct bintree* tree, const char* dumpfile_name);

void print_all_elements_beauty(struct bintreeElem* elem, int dump);

#endif