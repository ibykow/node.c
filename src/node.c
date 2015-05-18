#include "common.h"

#define node_table_full(n) (n->len == n->max)
#define node_table_resize(n) \
    (node_table_full(n) && !node_grow_table(n, n->max ? n->max << 1 : 2))

#if 0
struct data_s *data_copy(struct data_s *d)
{
    if(!d)
        return 0;

    struct data_s *copy = data_new(d->type, 0);
    if(!copy)
        return 0;

    copy->value = (void *) malloc(sizeof(copy->type->size));
    if(!copy->value) {
        free(copy);
        return 0;
    }

    memcpy(copy->value, d->value, copy->type->size);

    return copy;
}
#endif

static void node_free_table(struct node_s *n)
{
    while(n->len)
        node_free(n->table[--n->len]);

    free(n->table);
    n->table = 0;
    n->len = 0;
    n->max = 0;
}

static size_t node_resize_table(struct node_s *n, size_t size)
{
    if(!n || !size)
        return 0;
    struct node_s **new_table = (struct node_s **)
        realloc(n->table, sizeof(struct node_s *) * size);

    if(!new_table)
        return 0;

    n->table = new_table;
    n->max = size;

    return n->max;
}

void node_free(struct node_s *n)
{
    if(!n)
        return;

    if(n->table)
        node_free_table(n);

    if(n->owner)
        n->owner->table[n->id] = 0;

    n->type->freev(n->data);
    n->data = 0;
    free(n);
}

struct node_s *node_new(const struct node_type_s *type, const void *d,
    struct node_s *owner, size_t max)
{
    if(!d || !type)
        return 0;
    struct node_s *n = (struct node_s *) malloc(sizeof(struct node_s));
    if(!n)
        return 0;

    n->type = type;
    n->owner = 0;
    n->table = 0;
    n->id = 0;
    n->len = 0;
    n->max = max;

    if(!(n->data = type->new(d))) {
        free(n);
        return 0;
    }

    if(n->max &&
    !(n->table = (struct node_s **) malloc(sizeof(struct node_s *) * max))) {
        type->freev(n->data);
        free(n);
        return 0;
    }

    return n;
}

/*
 * size_t node_put(struct node_s *n, size_t index, struct node_s *new)
 *  Inserts a child node into a parent node at a given index.
 *
 *  Warning!!! This function replaces whatever was in the table at the index
 *  previously. It's your responsiblity to check and keep track of the node that
 *  might be present at the index.
 *
 * inputs:
 *  struct node_s *n - a pointer to the parent node
 *  size_t index - the index at which you want to insert the child node
 *  struct node_s *c - the index at which you want to insert the child node
 *
 * output:
 *  size_t - the new length of the parent node table, or 0 if an insertion
 *  did not take place.
 *
 * notes:
 *  - This function grows the parent node's table as necessary.
 *  - After insertion you can lookup the child node's current index in the
 *    parent's array by issuing c->id, where c is the pointer to the child node.
 */
size_t node_put(struct node_s *n, size_t index, struct node_s *c)
{
    if(!n || !c || (c->owner == n))
        return 0;

    if(index >= n->max && !node_resize_table(n, (index ? index : 1) << 1))
        return 0;

    if(index >= n->len) {
        memset(n->table + n->len, 0, (index - n->len) * sizeof(struct node_s *));
        n->len = index + 1;
    }

    n->table[index] = c;
    c->owner = n;
    c->id = index;

    return n->len;
}

/*
 * struct node_s *node_release(struct node_s *n, size_t index)
 *  Release and return a child node from a parent.
 *
 * inputs:
 *  struct node_s *n - a pointer to the node containing the child element you
 *  want released size_t index - the index of the child node within the parent
 *  nodes table.
 *
 * output:
 *  struct node_s * - a pointer to the child node if one exists, or 0 otherwise
 *
 * notes:
 *  - This function shrinks the parent node's table as necessary.
 *  - If you have a node pointer c belonging to another node n you can look up
 *      c's index in n's table by using c->id
 */
struct node_s *node_release(struct node_s *n, size_t index)
{
    struct node_s *ret = node_at(n, index);
    if(!ret)
        return 0;

    n->table[index] = 0;

    if(index == (n->len - 1)) {
        // Rewind to where there is an element
        while(n->len && !n->table[--n->len])
            ;

        // Resize the array accordingly
        if(!n->len)
            node_free_table(n);
        else if(n->len < (n->max >> 2))
            node_resize_table(n, n->len << 1);
    }

    ret->owner = 0;
    ret->id = 0;
    return ret;
}
