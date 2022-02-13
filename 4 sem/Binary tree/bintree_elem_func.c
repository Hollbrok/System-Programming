 #include "bintree_elem.h"

RET_ERR_TYPE createElem(struct bintreeElem* retElem, int value)
{
    if (retElem == NULL)
    {
        fprintf(stderr, "pointer to retElem in CREATE_ELEM is null.");
        return ERR_TREE_ELEM_NULL;
    }

    retElem->left_  = NULL;
    retElem->right_ = NULL;
    retElem->data_  = value;

    return ERR_SUCCESS;
}
