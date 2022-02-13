#ifndef BINTREE_H
#define BINTREE_h

#include "bintree_elem.h"

struct tree
{
	struct bintree_elem* root_  = nullptr;

    int size_ = 0;
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


#endif BINTREE_H