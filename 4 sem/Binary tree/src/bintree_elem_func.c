#include "../headers/bintree_elem.h"

static void *TEST_calloc(size_t nmemb, size_t size)
{
    static int randomErr = 1;

    if ( unlikely((randomErr++ % 55 == 0) || randomErr == 2) )
        return NULL;
    else
        return calloc(nmemb, size);
}

struct bintreeElem* createElem(int value)
{
    struct bintreeElem *newElem = (struct bintreeElem *) TEST_calloc(1, sizeof(struct bintreeElem));
    if (unlikely(newElem == NULL))
    {
        fprintf(stderr, "Can't calloc for elem in CREATE_ELEM.\n");
        return newElem;
    }

    newElem->data_  = value;

    return newElem;
}

void deconstrElem(struct bintreeElem* elem)
{
    free(elem);
}