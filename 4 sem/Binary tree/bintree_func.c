 #include "bintree.h"

RET_ERR_TYPE createTree(struct bintree* newTree)
{
    if (newTree == NULL)
    {
        fprintf(stderr, "pointer to newTree in CREATE_TREE is null.\n");
        return ERR_TREE_NULL;
    }

    newTree->root_ = NULL;
    newTree->size_ = 0;

    return ERR_SUCCESS;
}

RET_ERR_TYPE add(struct bintree* tree, int value)
{
    if (tree == NULL)
    {
        fprintf(stderr, "pointer to bintree in ADD is null.\n");
        return ERR_TREE_NULL; /* or exit? */
    }

    struct bintreeElem *newElem = (struct bintreeElem*) (calloc(1, sizeof(struct bintreeElem))); 
    if (newElem == NULL)
    {
        fprintf(stderr, "can't calloc memory for newElem in ADD.\n");
        return ERR_CALLOC;
    }

    enum ERRORS_TYPE retErrVal = -1;

    if ( (retErrVal = createElem(newElem, value)) != ERR_SUCCESS)
        return retErrVal;//exit(retVal);

    if (tree->root_ == NULL)
    {
        //fprintf(stderr, "root.\n");
        tree->root_ = newElem;
        tree->size_ = 1;
    }
    else /* there is at least root in tree */
    {
        //fprintf(stderr, "not a root.\n");
        retErrVal = addTo(tree->root_, newElem);

        if (retErrVal != ERR_ALREADY_EXISTS)
            tree->size_++;

        return retErrVal;
    }


    return ERR_SUCCESS;
}

RET_ERR_TYPE addTo(struct bintreeElem* mainElem, struct bintreeElem* insertElem)
{
    //fprintf(stderr, "DATA = %d.\n", insertElem->data_);

    if (insertElem->data_ < mainElem->data_)
    {
        if (mainElem->left_ == NULL)
        {
            //fprintf(stderr, "insert on the left %d.\n", insertElem->data_);
            mainElem->left_ = insertElem;
            //fprintf(stderr, "new left_");
        }
        else
        {
            //fprintf(stderr, "no null.\n");
            return addTo(mainElem->left_, insertElem);  
        }
    }
    else if (insertElem->data_ > mainElem->data_)
    {
        if (mainElem->right_ == NULL)
            mainElem->right_ = insertElem;
        else
            return addTo(mainElem->right_, insertElem);  
    }
    else 
    {
        //fprintf(stderr, "TETETTETE.\n");
        return ERR_ALREADY_EXISTS;
        /* equal => do nothing? */
    }


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
    fprintf(stderr, "NONONONONONONON.\n");

    if (mainElem->data_ == value)
    {
        
    }


}







/////////////////////////////////////////

void show_tree(struct bintree* tree)
{
    graphviz_beauty_dump(tree ,"dump/DUMP.dot");

    system("iconv -t UTF-8 -f  CP1251 < dump/DUMP.dot > dump/DUMP_temp.dot");
    system("dot dump/DUMP_temp.dot -Tpdf -o dump/DUMP.pdf");
    system("rm -rf dump/DUMP.dot");
    system("mv dump/DUMP_temp.dot dump/DUMP.dot");

    //system("okular DUMP.pdf");


    return;
}

void graphviz_beauty_dump(struct bintree* tree, const char* dumpfile_name)
{
    assert(dumpfile_name && "You passed nullptr dumpfile_name");

    int fileDump = open(dumpfile_name, 0666 | O_TRUNC, S_IRUSR | S_IWUSR | S_IWOTH | S_IROTH);
    if (fileDump == -1)
        ERR_HANDLER("open dumpfile");

    dprintf(fileDump, "digraph name {\n");
    dprintf(fileDump, "node [color = Red, fontname = Courier, style = filled, shape=ellipse, fillcolor = purple]\n");
    dprintf(fileDump, "edge [color = Blue, style=dashed]\n");

    print_all_elements_beauty(tree->root_, fileDump);

    dprintf(fileDump, "}//\n");

    close(fileDump);
    return;
}

void print_all_elements_beauty(struct bintreeElem* elem, int dumpFile)
{
    assert(elem && "elem is nullptr in print_all_elements");

    if (elem->left_ != NULL)
    {
        print_all_elements_beauty(elem->left_, dumpFile);
        dprintf(dumpFile, "\"%p\" -> \"%p\" [label=\"less\", fontcolor=darkblue]\n", elem, elem->left_);
    }
    if (elem->right_ != NULL)
    {
        print_all_elements_beauty(elem->right_, dumpFile);
        dprintf(dumpFile, "\"%p\" -> \"%p\" [label=\"more\", fontcolor=darkblue]\n", elem, elem->right_);
    }

    if ((elem->right_ == NULL) && (elem->left_ == NULL))
        dprintf(dumpFile, "\"%p\" [label = \"%d\",style = filled, fillcolor = lightgreen] \n", elem, elem->data_);
    else
        dprintf(dumpFile, "\"%p\" [label = \"%d\",style = filled, fillcolor = purple] \n", elem, elem->data_);
    return;
}
