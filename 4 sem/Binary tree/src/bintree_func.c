 #include "../headers/bintree.h"

void *TEST_calloc(size_t nmemb, size_t size)
{
    static int randomErr = 1;

    if ( randomErr++ % 50 == 0)
    {
        return NULL;
    }
    else
    {
        return calloc(nmemb, size);
    }
}


RET_ERR_TYPE createTree(struct bintree* newTree)
{
    printf("01\n");
    printf("[%p]\n", newTree);
    if (newTree == NULL)
    {
        printf("02\n");
        fprintf(stderr, "pointer to newTree in CREATE_TREE is null.\n");
        return ERR_TREE_NULL;
    }
    
    printf("1");
    newTree->root_ = NULL;
    printf("2");
    newTree->size_ = 0;
    printf("3\n");

    return ERR_SUCCESS;
}

RET_ERR_TYPE add(struct bintree* tree, int value)
{
    if (tree == NULL)
    {
        fprintf(stderr, "pointer to bintree in ADD is null.\n");
        return ERR_TREE_NULL; /* or exit? */
    }

    struct bintreeElem *newElem = (struct bintreeElem*) (TEST_calloc(1, sizeof(struct bintreeElem))); 
    if (newElem == NULL)
    {
        fprintf(stderr, "can't calloc memory for newElem in ADD.\n");
        return ERR_CALLOC;
    }

    enum ERRORS_TYPE retErrVal = -1;

    createElem(newElem, value); /* can't return any error if newElem isn't NULL*/

    if (tree->root_ == NULL)
    {
        fprintf(stderr, "root is null.\n");
        tree->root_ = newElem;
        tree->size_ = 1;
    }
    else /* there is at least root in tree */
    {
        retErrVal = addTo(tree->root_, newElem);

        if (retErrVal == ERR_SUCCESS)
            tree->size_++;
        else
        {
            free(newElem);
            return retErrVal;
        }
    }


    return ERR_SUCCESS;
}

RET_ERR_TYPE addTo(struct bintreeElem* mainElem, struct bintreeElem* insertElem)
{
    if (insertElem->data_ < mainElem->data_)
    {
        if (mainElem->left_ == NULL)
            mainElem->left_ = insertElem;
        else
            return addTo(mainElem->left_, insertElem);  
    }
    else if (insertElem->data_ > mainElem->data_)
    {
        if (mainElem->right_ == NULL)
            mainElem->right_ = insertElem;
        else
            return addTo(mainElem->right_, insertElem);  
    }
    else 
        return ERR_ALREADY_EXISTS;


    return ERR_SUCCESS;
}

RET_ERR_TYPE removeElem(struct bintree* tree, int value)
{
    if (tree == NULL)
    {
        fprintf(stderr, "pointer to bintree in REMOVE_ELEM is null.\n");
        return ERR_TREE_NULL; /* or exit? */
    }

    if (tree->root_ == NULL)
    {
        return ERR_EMPTY_TREE;
    }
    else
    {
        enum ERRORS_TYPE retVal = ERROR;

        if (tree->root_->data_ == value)
        {
            struct bintreeElem *saveRoot = tree->root_;

            if (tree->root_->right_ != NULL)
            {
                if (tree->root_->left_ != NULL)
                {
                    struct bintreeElem* iterElem = tree->root_->right_;
                    
                    while (iterElem->left_ != NULL)
                        iterElem = iterElem->left_;

                    iterElem->left_ = tree->root_->left_;
                }

                tree->root_ = tree->root_->right_;
            }
            else if (tree->root_->left_ != NULL)
                tree->root_ = tree->root_->left_;

            deconstrElem(saveRoot);
            tree->size_--;

            if (tree->size_ == 0)
                tree->root_ = NULL;
            
        }
        else /* check if there are left or|and right|left elems*/
        {
            if (value < tree->root_->data_)
            { 
                if ( (tree->root_->left_ != NULL) && (value != tree->root_->left_->data_))
                    retVal = removeElemFrom(tree->root_->left_, value);
                else if (tree->root_->left_ != NULL)
                {
                    struct bintreeElem *saveLeft = tree->root_->left_;

                    if (tree->root_->left_->right_ != NULL)
                    {
                        struct bintreeElem *iterElem = tree->root_->left_->right_;
                        while (iterElem->left_ != NULL)
                            iterElem = iterElem->left_;

                        iterElem->left_ = tree->root_->left_->left_;

                        tree->root_->left_ = saveLeft->right_;

                    }
                    else if (tree->root_->left_->left_ != NULL)
                        tree->root_->left_ = tree->root_->left_->left_;
                    else
                    {
                        tree->root_->left_ = NULL;
                    }
                    
                    deconstrElem(saveLeft);
                }
            }
            else
            {
                if ( (tree->root_->right_ != NULL) && (value != tree->root_->right_->data_))
                    retVal = removeElemFrom(tree->root_->right_, value);
                else if (tree->root_->right_ != NULL)
                {
                    struct bintreeElem *saveRight = tree->root_->right_;

                    if (tree->root_->right_->right_ != NULL)
                    {
                        struct bintreeElem *iterElem = tree->root_->right_->right_;
                        while (iterElem->left_ != NULL)
                            iterElem = iterElem->left_;

                        iterElem->left_ = tree->root_->right_->left_;

                        tree->root_->right_ = saveRight->right_;

                    }
                    else if (tree->root_->right_->left_ != NULL)
                        tree->root_->right_ = tree->root_->right_->left_;
                    else
                    {
                        tree->root_->right_ = NULL;
                    }
                    
                    deconstrElem(saveRight);
                }
            }
            if (retVal != ERR_NO_NEED_ELEM)
                tree->size_--;
        }
    }

    return ERR_SUCCESS;
}

/* remove element with data = value, but starting from mainElem (which is not included)*/
RET_ERR_TYPE removeElemFrom(struct bintreeElem* mainElem, int value)
{
    if (value < mainElem->data_)
    { 
        if ( (mainElem->left_ != NULL) && (value != mainElem->left_->data_))
            return removeElemFrom(mainElem->left_, value);
        else if (mainElem->left_ != NULL)
        {
            struct bintreeElem *saveLeft = mainElem->left_;
            if (mainElem->left_->right_ != NULL)
            {
                struct bintreeElem *iterElem = mainElem->left_->right_;
                while (iterElem->left_ != NULL)
                    iterElem = iterElem->left_;

                iterElem->left_ = mainElem->left_->left_;
                mainElem->left_ = saveLeft->right_;
            }
            else if (mainElem->left_->left_ != NULL)
                mainElem->left_ = mainElem->left_->left_;
            else
            {
                mainElem->left_ = NULL;
            }
                
            deconstrElem(saveLeft);
        }
    }
    else
    {
        if ( (mainElem->right_ != NULL) && (value != mainElem->right_->data_))
            return removeElemFrom(mainElem->right_, value);
        else if (mainElem->right_ != NULL)
        {
            struct bintreeElem *saveRight = mainElem->right_;
            if (mainElem->right_->right_ != NULL)
            {
                struct bintreeElem *iterElem = mainElem->right_->right_;
                while (iterElem->left_ != NULL)
                    iterElem = iterElem->left_;

                iterElem->left_ = mainElem->right_->left_;
                mainElem->right_ = saveRight->right_;
            }
            else if (mainElem->right_->left_ != NULL)
                mainElem->right_ = mainElem->right_->left_;
            else
            {
                mainElem->right_ = NULL;
            }
                
            deconstrElem(saveRight);
        }
    }

}

RET_ERR_TYPE clear(struct bintree* tree)
{
    if (tree == NULL)
    {
        fprintf(stderr, "pointer to bintree in CLEAR is null.\n");
        return ERR_TREE_NULL; /* or exit? */
    }

    if (tree->root_ != NULL && tree->size_ != 0)
    {
        clearFrom(tree->root_);
                
        tree->size_ = 0;
        tree->root_ = NULL;
        return ERR_SUCCESS;        
    }
    else
    {
        fprintf(stderr, "root is NULL in CLEAR.\n");
        return ERR_TREE_ELEM_NULL;
    }
}

void clearFrom(struct bintreeElem* mainElem)
{
    enum ERRORS_TYPE retVal = ERROR;

    if (mainElem->left_ != NULL)
        clearFrom(mainElem->left_);
    if (mainElem->right_ != NULL)
        clearFrom(mainElem->right_);
    
    deconstrElem(mainElem);
}

int search(struct bintree* tree, int value)
{
    if (tree == NULL)
    {
        fprintf(stderr, "pointer to bintree in SEARCH is null.\n");
        return ERR_TREE_NULL; /* or exit? */
    }

    if (tree->root_ != NULL)
        return searchFrom(tree->root_, value);
    else
        return ERR_TREE_ELEM_NULL;
}

RET_ERR_TYPE searchFrom(struct bintreeElem* mainElem, int value)
{
    if (mainElem->data_ == value)
        return ERR_SUCCESS;
    else
    {
        enum ERRORS_TYPE retVal = ERROR;

        if (mainElem->left_ != NULL)
            if ( (retVal = searchFrom(mainElem->left_, value) ) == ERR_SUCCESS)
                return ERR_SUCCESS;
        if (mainElem->right_ != NULL)
            if ( (retVal = searchFrom(mainElem->right_, value) ) == ERR_SUCCESS)
                return ERR_SUCCESS;
        return ERROR;
    }
}

/* ORDETING foreach */

RET_ERR_TYPE foreach(enum ORDERING_TYPE orderType, struct bintree *tree, int (func)(struct bintreeElem *, void *), void *x)
{
    if (tree == NULL)
    {
        fprintf(stderr, "pointer to bintree in FOREACH is null.\n");
        return ERR_TREE_NULL; /* or exit? */
    }

    if (tree->root_ == NULL)
    {
        fprintf(stderr, "pointer to root in FOREACH is null.\n");
        return ERR_TREE_ELEM_NULL; /* or exit? */
    }

    if (func == NULL)
    {
        fprintf(stderr, "pointer to func in FOREACH is null.\n");
        return ERR_FUNC_NULL; /* or exit? */
    }

    return foreachFrom(orderType, tree->root_, func, x);
}

RET_ERR_TYPE foreachFrom(enum ORDERING_TYPE orderType, struct bintreeElem *mainElem, int (func)(struct bintreeElem *, void *), void *x)
{
    switch (orderType)
    {
    case OR_T_PREORDER:
        func(mainElem, x);

        if (mainElem->left_ != NULL)
            foreachFrom(orderType, mainElem->left_, func, x);
        if (mainElem->right_ != NULL)
            foreachFrom(orderType, mainElem->right_, func, x);
        break;
    case OR_T_POSTORDER:
        if (mainElem->left_ != NULL)
            foreachFrom(orderType, mainElem->left_, func, x);
        if (mainElem->right_ != NULL)
            foreachFrom(orderType, mainElem->right_, func, x);

        func(mainElem, x);
        break;
    case OR_T_INORDER:
        if (mainElem->left_ != NULL)
            foreachFrom(orderType, mainElem->left_, func, x);

        func(mainElem, x);

        if (mainElem->right_ != NULL)
            foreachFrom(orderType, mainElem->right_, func, x);
        break;
    default: /* POSTORDER by default */
        return foreachFrom(OR_T_POSTORDER, mainElem, func, x);
        break;
    }

    return ERR_SUCCESS;
}



/////////////////////////////////////////

void show_tree(struct bintree* tree)
{
    if (tree == NULL)
    {
        fprintf(stderr, "null tree in show_tree.\n");
        return;
    }

    fprintf(stderr, "TEST1\n");
    graphviz_beauty_dump(tree ,"dump/DUMP.dot");
    fprintf(stderr, "TEST3\n");

    system("iconv -t UTF-8 -f  CP1251 < dump/DUMP.dot > dump/DUMP_temp.dot");
    system("dot dump/DUMP_temp.dot -Tpdf -o dump/DUMP.pdf");
    system("rm -rf dump/DUMP.dot");
    system("mv dump/DUMP_temp.dot dump/DUMP.dot");
 

    return;
}

void graphviz_beauty_dump(struct bintree* tree, const char* dumpfile_name)
{
    assert(dumpfile_name && "You passed nullptr dumpfile_name");

    mkdir("dump", S_IRWXU | S_IRWXG | S_IROTH);

    int fileDump = open(dumpfile_name, 0666 | O_TRUNC, S_IRWXU | S_IRWXG | S_IROTH);
    if (fileDump == -1 && errno != EEXIST)
        ERR_HANDLER("open dumpfile");

    dprintf(fileDump, "digraph name {\n");
    dprintf(fileDump, "node [color = Red, fontname = Courier, style = filled, shape = ellipse, fillcolor = purple]\n");
    dprintf(fileDump, "edge [color = Blue, style = dashed]\n");

    fprintf(stderr, "TEST2\n");

    if (tree->root_ != NULL)
    {
        fprintf(stderr, "TEST ROOT IS NOT A NULL\n");
        print_all_elements_beauty(tree->root_, fileDump);
    }
    else
        fprintf(stderr, "NULL root in DUMP-func.\n");

    dprintf(fileDump, "}//\n");

    close(fileDump);
    return;
}

void print_all_elements_beauty(struct bintreeElem* elem, int dump)
{
    assert(elem && "elem is nullptr in print_all_elements");

    if (elem->left_ != NULL)
    {
        print_all_elements_beauty(elem->left_, dump);
        dprintf(dump, "\"%p\" -> \"%p\" [label=\"less\", fontcolor=darkblue]\n", elem, elem->left_);
    }
    if (elem->right_ != NULL)
    {
        print_all_elements_beauty(elem->right_, dump);
        dprintf(dump, "\"%p\" -> \"%p\" [label=\"more\", fontcolor=darkblue]\n", elem, elem->right_);
    }

    if ((elem->right_ == NULL) && (elem->left_ == NULL))
        dprintf(dump, "\"%p\" [label = \"%d\",style = filled, fillcolor = lightgreen] \n", elem, elem->data_);
    else
        dprintf(dump, "\"%p\" [label = \"%d\",style = filled, fillcolor = purple] \n", elem, elem->data_);
    return;
}
