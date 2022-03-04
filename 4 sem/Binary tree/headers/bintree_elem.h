#ifndef BINTREE_ELEM_H
#define BINTREE_ELEM_H

#include "libs.h"


struct bintreeElem
{
	struct bintreeElem* left_ ;// = nullptr;
	struct bintreeElem* right_;// = nullptr;

    int data_;// = 0;
};

/* */

void *TEST_calloc(size_t nmemb, size_t size);

RET_ERR_TYPE initElem(struct bintreeElem* retElem, int value);

struct bintreeElem* createElem(int value);


void deconstrElem(struct bintreeElem* elem);

#endif