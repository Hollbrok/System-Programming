#include "../headers/bintree.h"

static RET_ERR_TYPE addTo(struct bintreeElem* mainElem, struct bintreeElem* insertElem);

static RET_ERR_TYPE removeElemFrom(struct bintreeElem* mainElem, int value);

static RET_ERR_TYPE removeRoot(struct bintree *tree);

static RET_ERR_TYPE clearFrom(struct bintreeElem* mainElem);

static RET_ERR_TYPE searchFrom(struct bintreeElem* mainElem, int value);

static RET_ERR_TYPE foreachFrom(enum ORDERING_TYPE orderType, struct bintreeElem *mainElem, int (func)(struct bintreeElem *, void *), void *x);


static void graphviz_beauty_dump(struct bintree* tree, const char* dumpfile_name);

static int printInfo(struct bintreeElem *elem, void *infoDump);

static void print_all_elements_beauty(struct bintree* tree, int dump);



static void *TEST_calloc(size_t nmemb, size_t size)
{
    static int randomErr = 1;

    if ( unlikely((randomErr++ % 55 == 0) || randomErr == 2) )
        return NULL;
    else
        return calloc(nmemb, size);
}

struct bintree* createTree()
{
    return (struct bintree *) TEST_calloc(1, sizeof(struct bintree));
}

RET_ERR_TYPE add(struct bintree* tree, int value)
{
    if (unlikely(tree == NULL))
    {
        fprintf(stderr, "pointer to bintree in ADD is null.\n");
        return ERR_TREE_NULL; /* or exit? */
    }

    struct bintreeElem *newElem = createElem(value);
    if (unlikely(newElem == NULL))
        return ERR_CALLOC;

    enum ERRORS_TYPE retErrVal = ERR_SUCCESS;

    retErrVal = addTo(tree->root_, newElem);

    if (retErrVal == ERR_TREE_ELEM_NULL) /* there is only 1 possibility: root in NULL */
    {
        tree->root_ = newElem;
        tree->size_++;
        return ERR_SUCCESS;
    }

    if (likely(retErrVal == ERR_SUCCESS))
        tree->size_++;
    else
        free(newElem);

    return retErrVal;
}

/* this method should be called only from method add(struct bintree* tree, int value) .*/
static RET_ERR_TYPE addTo(struct bintreeElem* mainElem, struct bintreeElem* insertElem)
{
    if (unlikely(mainElem == NULL || insertElem == NULL))
        return ERR_TREE_ELEM_NULL;

    if (insertElem->data_ == mainElem->data_)
        return ERR_ALREADY_EXISTS;
    
    
    if (insertElem->data_ < mainElem->data_)
    {
        if (mainElem->left_ != NULL)
            return addTo(mainElem->left_, insertElem);
        else
            mainElem->left_ = insertElem;  
    }
    else if (insertElem->data_ > mainElem->data_)
    {
        if (mainElem->right_ != NULL)
            return addTo(mainElem->right_, insertElem);  
        else
            mainElem->right_ = insertElem;
    }


    return ERR_SUCCESS;
}


RET_ERR_TYPE removeElem(struct bintree *tree, int value)
{
    if (unlikely(tree == NULL))
    {
        fprintf(stderr, "pointer to bintree in REMOVE_ELEM is null.\n");
        return ERR_TREE_NULL; /* or exit? */
    }

    if (unlikely(tree->root_ == NULL))
        return ERR_EMPTY_TREE;

    if (unlikely(tree->root_->data_ == value))
        return removeRoot(tree); 

    enum ERRORS_TYPE retVal = ERROR;
    if ( (retVal = removeElemFrom(tree->root_, value)) == ERR_SUCCESS)
            tree->size_--;

    return retVal;
}

/* remove element with data = value, but starting from mainElem (which is not included)*/
static RET_ERR_TYPE removeElemFrom(struct bintreeElem* mainElem, int value)
{
    if (unlikely(mainElem == NULL))
    {
        fprintf(stderr, "NULL pointer to mainElem in REMOVE_ELEM_FROM.\n");
        return ERR_TREE_ELEM_NULL;
    }


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
                mainElem->left_ = NULL;

            deconstrElem(saveLeft);
            return ERR_SUCCESS;
        }
    }

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
            mainElem->right_ = NULL;
            
        deconstrElem(saveRight);
        return ERR_SUCCESS;

    }
    else
        return ERR_NO_NEED_ELEM;
}

static RET_ERR_TYPE removeRoot(struct bintree *tree)
{
    struct bintreeElem *saveRoot = tree->root_;

    if ( (tree->root_->right_ != NULL) && (tree->root_->left_ != NULL) )
    {
        struct bintreeElem* iterElem = tree->root_->right_;
                    
        while (iterElem->left_ != NULL)
            iterElem = iterElem->left_;

        iterElem->left_ = tree->root_->left_;

        tree->root_ = tree->root_->right_;
    }
    else if (tree->root_->left_ != NULL)
        tree->root_ = tree->root_->left_;

    deconstrElem(saveRoot);
    tree->size_--;

    if (tree->size_ == 0)
        tree->root_ = NULL;
        
        return ERR_SUCCESS;  
}

RET_ERR_TYPE clear(struct bintree* tree)
{
    if (unlikely(tree == NULL))
    {
        fprintf(stderr, "pointer to bintree in CLEAR is null.\n");
        return ERR_TREE_NULL; /* or exit? */
    }

    if (tree->root_ != NULL && tree->size_ != 0)
    {
        enum ERRORS_TYPE retVal = clearFrom(tree->root_);
                
        tree->size_ = 0;
        tree->root_ = NULL;
        return retVal;        
    }

    fprintf(stderr, "root is NULL in CLEAR.\n");
    return ERR_TREE_ELEM_NULL;
}

static RET_ERR_TYPE clearFrom(struct bintreeElem* mainElem)
{
    if (unlikely(mainElem == NULL))
    {
        //fprintf(stderr, "NULL pointer to mainElem in CLEAR_FROM.\n");
        return ERR_TREE_ELEM_NULL;
    }

    clearFrom(mainElem->left_);
    clearFrom(mainElem->right_);

    deconstrElem(mainElem);
    return ERR_SUCCESS;
}

RET_ERR_TYPE search(struct bintree* tree, int value)
{
    if (unlikely(tree == NULL))
    {
        fprintf(stderr, "pointer to bintree in SEARCH is null.\n");
        return ERR_TREE_NULL; /* or exit? */
    }

    return searchFrom(tree->root_, value);
}

static RET_ERR_TYPE searchFrom(struct bintreeElem* mainElem, int value)
{
    if (unlikely(mainElem == NULL))
    {
        fprintf(stderr, "NULL pointer to mainElem in SEARCH_FROM.\n");
        return ERR_TREE_ELEM_NULL;
    }

    for(; (mainElem->left_ != NULL) || (mainElem->right_ != NULL) ;)
    {
        if (mainElem->data_ == value)
            return ERR_SUCCESS;

        if ( (mainElem->left_ != NULL) && (value < mainElem->data_) )
            mainElem = mainElem->left_;

        else if ( (mainElem->right_ != NULL) && (value > mainElem->data_) )
            mainElem = mainElem->right_;
    }

}

/* Ordering foreach */

RET_ERR_TYPE foreach(enum ORDERING_TYPE orderType, struct bintree *tree, int (func)(struct bintreeElem *, void *), void *x)
{
    if (unlikely(tree == NULL))
    {
        fprintf(stderr, "pointer to bintree in FOREACH is null.\n");
        return ERR_TREE_NULL;
    }

    if (unlikely(func == NULL))
    {
        fprintf(stderr, "pointer to func in FOREACH is null.\n");
        return ERR_FUNC_NULL;
    }

    return foreachFrom(orderType, tree->root_, func, x);
}

static RET_ERR_TYPE foreachFrom(enum ORDERING_TYPE orderType, struct bintreeElem *mainElem, int (func)(struct bintreeElem *, void *), void *x)
{
    if (unlikely(mainElem == NULL))
    {
        fprintf(stderr, "NULL pointer to mainElem in FOREACH_FROM.\n");
        return ERR_TREE_ELEM_NULL;
    }

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
    if (unlikely(tree == NULL))
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

static void graphviz_beauty_dump(struct bintree* tree, const char* dumpfile_name)
{
    assert(dumpfile_name && "You passed nullptr dumpfile_name");

    mkdir("dump", S_IRWXU | S_IRWXG | S_IROTH);

    int fileDump = open(dumpfile_name, 0666 | O_TRUNC, S_IRWXU | S_IRWXG | S_IROTH);
    if (unlikely(fileDump == -1 && errno != EEXIST))
        ERR_HANDLER("open dumpfile");

    dprintf(fileDump, "digraph name {\n");
    dprintf(fileDump, "node [color = Red, fontname = Courier, style = filled, shape = ellipse, fillcolor = purple]\n");
    dprintf(fileDump, "edge [color = Blue, style = dashed]\n");

    fprintf(stderr, "TEST2\n");

    if (likely(tree->root_ != NULL))
    {
        fprintf(stderr, "TEST ROOT IS NOT A NULL\n");
        print_all_elements_beauty(tree, fileDump);
    }
    else
        fprintf(stderr, "NULL root in DUMP-func. size = %d, root = %p\n", tree->size_, tree->root_);

    dprintf(fileDump, "}//\n");

    close(fileDump);
    return;
}


static int printInfo(struct bintreeElem *elem, void *infoDump)
{
    int fd = *((int *)infoDump);

    if (likely(elem->left_ != NULL))
    {
        dprintf(fd, "\"%p\" -> \"%p\" [label=\"less\", fontcolor=darkblue]\n", elem, elem->left_);
    }
    if (likely(elem->right_ != NULL))
    {
        dprintf(fd, "\"%p\" -> \"%p\" [label=\"more\", fontcolor=darkblue]\n", elem, elem->right_);
    }

    if ((elem->right_ == NULL) && (elem->left_ == NULL))
        dprintf(fd, "\"%p\" [label = \"%d\",style = filled, fillcolor = lightgreen] \n", elem, elem->data_);
    else
        dprintf(fd, "\"%p\" [label = \"%d\",style = filled, fillcolor = purple] \n", elem, elem->data_);

    return 0;

}

static void print_all_elements_beauty(struct bintree* tree, int dumpFd)
{
    assert(tree && "elem is nullptr in print_all_elements");

    foreach(OR_T_PREORDER, tree, printInfo, &dumpFd);

    return;
}