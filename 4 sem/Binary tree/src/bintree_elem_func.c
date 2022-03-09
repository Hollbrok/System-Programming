#include "../headers/bintree_elem.h"

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