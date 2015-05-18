/*
 * node.c
 * Ilia Bykow, 2015
 *
 * Create generic, associative data types and structures.
 *
 * Nodes can act as vertices, edges, weighted edges, associative arrays
 * and so on. The undrelying table structure is a dynamically resized array.
 *
 * All allocations and memory management is done internally, either here,
 * or by functions specified by the node's type structure. This means
 * that whether you're creating, modifying or destroying nodes you must use
 * the handler functions provided.
 *
 * See str.c and str.h for an example of how to create a custom node
 * structure and its associated node-type.
 */

#include "common.h"

/*
 * Macros
 */
#define node_table_full(n) (n->len == n->max)

/*
 * Clear a certain range of the node's table
 */
#define node_clear_table(n, from, to) \
    memset((n)->table + (from), 0, (to) * sizeof(struct node_s *))

/*
 * Frees nested nodes.
 */
static void nested_node_free(void *data)
{
    if(!data)
        return;

    /*
     * Recurse as necessary.
     */
    if(nested_node_freeit(data))
        node_free(nested_node_node(data));

    /*
     * Free the nested_node_s structure which was allocated
     * in _nested_node_new.
     */
    free(data);
}

/*
 * Referenced by nested_node_type.
 */
static void *_nested_node_new(const void *init)
{
    if(!init)
        return 0;

    struct nested_node_s *nn = (struct nested_node_s *)
        malloc(sizeof(struct nested_node_s));

    if(!nn)
        return 0;

    nn->node = (void *) nested_node_node(init);
    nn->freeit = nested_node_freeit(init);

    return (void *) nn;
}

/*
 * nested_node_diff must exist as a function because it is referenced
 * by nested_node_type.
 */
static int nested_node_diff(const void *a, const void *b)
{
    return node_diff((struct node_s *) nested_node_node(a),
                    (struct node_s *) nested_node_node(b));
}

/*
 * Frees all child elements.
 */
static void node_free_table(struct node_s *n)
{
    while(n->len)
        node_free(n->table[--n->len]);

    free(n->table);

    n->table = 0;
    n->len = 0;
    n->max = 0;
}

/*
 * Either shrinks or grows the table.
 */
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

/*
 * Check to see if the table needs shrinking
 * and shrink accordingly.
 */
static size_t node_tighten_table(struct node_s *n)
{
    /*
     * If n->max is 0 we cannot tighten any further.
     */
    if(!n->max)
        return 0;

    /*
     * Rewind back to the previous available element.
     */
    for(; n->len && !n->table[n->len - 1]; n->len--)
        ;

    /*
     * Use a 4 to 2 threshold: if we have 1/4 the elements,
     * shrink the table by half.
     */
    if(!n->len)
        node_free_table(n);
    else if(n->len < (n->max >> 2))
        node_resize_table(n, n->len << 1);

    return n->max;
}

/*
 * Remove the node from its owner's table if an owner exists.
 * If an owner does exist, assume that it is a valid node
 * and that it remembers us (ie. that its table has a record of us).
 */
static void node_emancipate(struct node_s *n)
{
    /*
     * If we don't belong to anyone, there's nothing to do.
     */
    if(!n->owner)
        return;

    /*
     * Remove ourselves from the owner's table *before* we
     * attempt to tighten it.
     */
    n->owner->table[n->id] = 0;

    /*
     * Run the tightening operation because we may have cleared
     * up enough room in the owner's table for it to be worth it.
     */
    node_tighten_table(n->owner);

    /*
     * Forget the owner and clear our id.
     */
    n->owner = 0;
    n->id = 0;
}

/*
 * Move the child element from its current owner (if any) to a new one.
 * index specifies the position of the child element withing the new
 * owner's table.
 */
static struct node_s *node_adopt(struct node_s *n, struct node_s *c, size_t index)
{
    /*
     * Dissociate the child from the owner.
     */
    node_emancipate(c);

    /*
     * Adopt the new child and update the child's id.
     */
    n->table[index] = c;
    c->owner = n;
    c->id = index;

    return c;
}

/*
 * Free the current node, its value and recursively free
 * all of its children and their values and so on.
 */
void node_free(struct node_s *n)
{
    /*
     * Sanity check to make sure there is a node being passed in.
     */
    if(!n)
        return;

    /*
     * After this, all the children along with the table itself
     * will have been freed.
     */
    if(n->table)
        node_free_table(n);

    /*
     * Remove ourselves from any owner nodes.
     */
    node_emancipate(n);

    /*
     * Finally, we free our own data, the pointer to it, and ourselves.
     * The 'freev' function provided by the type must handle freeing all
     * its data internals up to and including n->data itself as it sees fit.
     */
    n->type->freev(n->data);
    n->data = 0;
    free(n);
}

/*
 * Create a new node given its type and a const representation of its data.
 */
struct node_s *node_new(const struct node_type_s *type, const void *d)
{
    /*
     * Sanitize the input, so we don't waste time with null pointers.
     */
    if(!d || !type)
        return 0;

    /*
     * Allocate the node structure.
     */
    struct node_s *n = (struct node_s *) malloc(sizeof(struct node_s));
    if(!n)
        return 0;

    /*
     * Create the correct data structure and populate it using
     * the initial data provided.
     */
    if(!(n->data = type->new(d))) {
        free(n);
        return 0;
    }

    /*
     * Initialize the node structure.
     */
    n->type = type;
    n->owner = 0;
    n->table = 0;
    n->id = 0;
    n->len = 0;
    n->max = 0;

    return n;
}

/*
 * Use this function instead of calling a node's diff directly.
 * We can quickly determine equality or inequality based on pointer,
 * and node type.
 */
int node_diff(const struct node_s *a, const struct node_s *b)
{
    /*
     * Two null pointers will be considered equal.
     */
    if(a == b)
        return 0;

    /*
     * If only one pointer is null, or the nodes are of different type
     * return a standard diff.
     */
    if((!a || !b) || (a->type != b->type))
        return NODE_TYPE_DIFF;

    return a->type->diff(a->data, b->data);
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
    /*
     * Make sure the table has enough room.
     */
    if( !n || !c || (c->owner == n) ||
        (index >= n->max && !node_resize_table(n, (index ? index : 1) << 1)))
        return 0;

    /*
     * Clear all pointers leading up from the previous set element.
     */
    if(index >= n->len) {
        node_clear_table(n, n->len, index - n->len);
        n->len = index + 1;
    }

    node_adopt(n, c, index);

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
    /*
     * Lookup the data. node_at will do bounds and sanity checks.
     */
    struct node_s *ret = node_at(n, index);

    /*
     * Only emancipate if we have something.
     */
    if(ret)
        node_emancipate(ret);

    /*
     * Return either the emancipated node, or nothing if a node
     * was not present.
     */
    return ret;
}

/*
 * Define the node_type_s structure that will be used to create
 * nested nodes (nodes with other nodes as their data).
 */
static const struct node_type_s _nested_node_type = {
    .size = sizeof(struct nested_node_s),
    .freev = nested_node_free,
    .new = _nested_node_new,
    .diff = nested_node_diff
};

/*
 * Create a const pointer to the above structure which can then
 * be passed to the node_new function.
 */
const struct node_type_s *nested_node_type = &_nested_node_type;
