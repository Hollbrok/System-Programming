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


//// DUMP

void show_tree(struct bintree* tree);
void graphviz_beauty_dump(struct bintree* tree, const char* dumpfile_name);
void print_all_elements_beauty(struct bintreeElem* elem, int dump);

#endif