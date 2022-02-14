#ifndef BINTREE_ELEM_H
#define BINTREE_ELEM_H

#include "libs.h"


struct bintreeElem
{
	struct bintreeElem* left_ ;// = nullptr;
	struct bintreeElem* right_;// = nullptr;

    int data_;// = 0;
};

/////

/*

    create_elem 

*/

RET_ERR_TYPE createElem(struct bintreeElem* retElem, int value);

RET_ERR_TYPE deconstrElem(struct bintreeElem* elem);

#endif
