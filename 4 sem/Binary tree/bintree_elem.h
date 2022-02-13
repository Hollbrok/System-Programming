#ifndef BINTREE_ELEM_H
#define BINTREE_ELEM_H

#include "libs.h"

struct bintree_elem
{
	struct bintree_elem* left_  = nullptr;
	struct bintree_elem* right_ = nullptr;

    int data_ = 0;
};

/////



#endif BINTREE_ELEM_H