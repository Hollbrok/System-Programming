#include "../headers/bintree_elem.h"

RET_ERR_TYPE initElem(struct bintreeElem* retElem, int value)
{
    if (retElem == NULL)
    {
        fprintf(stderr, "pointer to retElem in CREATE_ELEM is null.\n");
        return ERR_TREE_ELEM_NULL;
    }

    retElem->left_  = NULL;
    retElem->right_ = NULL; 
    retElem->data_  = value;

    return ERR_SUCCESS;
}

struct bintreeElem* createElem(int value)
{
    struct bintreeElem *newElem = (struct bintreeElem *) TEST_calloc(1, sizeof(struct bintreeElem));
    if (newElem == NULL)
    {
        fprintf(stderr, "Can't calloc for elem in CREATE_ELEM.\n");
        return newElem;
    }

    newElem->left_  = NULL;
    newElem->right_ = NULL; 
    newElem->data_  = value;

    return newElem;
}

RET_ERR_TYPE deconstrElem(struct bintreeElem* elem)
{
    if (elem == NULL)
    {
        fprintf(stderr, "pointer to elem in REMOVE is null.\n");
        return ERR_TREE_ELEM_NULL; /* or exit? */
    }
    else
    {
        elem->left_  = NULL;
        elem->right_ = NULL;        
        elem->data_  = 0;

        free(elem);
        elem = NULL;

        return ERR_SUCCESS;
    }


}