 #include "../headers/bintree_elem.h"

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

        fprintf(stderr, "2");
        free(elem);

        return ERR_SUCCESS;
    }


}