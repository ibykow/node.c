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
// #define PR_DEBUG
#include "common.h"

/*
 * macros
 */
#define node_table_full(n) (n->len == n->max)

/*
 * node_clear_table
 * Clears a certain range of the node's table
 */
#define node_clear_table(n, from, to) \
    memset((n)->table + (from), 0, (to) * sizeof(struct node_s *))

/*
 * static functions
 */

static void _node_free(void *d)
{
    node_free_all((struct node_s *) d);
}

static void *_node_new(const void *d)
{
    return (void *) d;
}

static int _node_diff(const void *a, const void *b)
{
    return node_diff((struct node_s *) a, (struct node_s *) b);
}

static struct node_s *to_str(const void *data)
{
    // return node_to_str((struct node_s *) data);
    return 0;
}

/*
 * static void node_free_table(struct node_s *n, bool recurse);
 * Frees all child elements.
 */
static void node_free_table(struct node_s *n, bool recurse)
{
    unsigned i;
    for(i = 0; i < n->len; i++)
        node_free(n->table[i], recurse);

    pr_dbg("%p (%p)", n->table, n);
    free(n->table);

    n->table = 0;
    n->len = 0;
    n->max = 0;
}

/*
 * static size_t node_resize_table(struct node_s *n, size_t size)
 * Resize the table to size greater than 0. If we want to resize
 * the table to 0, we use node_free_table instead.
 */
static size_t node_resize_table(struct node_s *n, size_t size)
{
    if(!n || !size)
        return 0;

    pr_dbg("n: %p, n->table: %p, size: %lu", n, n->table, size);

    struct node_s **new_table = (struct node_s **)
        realloc(n->table, sizeof(struct node_s *) * size);
    if(!new_table)
        return 0;

    pr_dbg("n->table: %p, new_table: %p", n->table, new_table);

    n->table = new_table;
    n->max = size;

    return n->max;
}

/*
 * static size_t node_tighten_table(struct node_s *n, bool recurse);
 * Shrink the table if necessary.
 */
static size_t node_tighten_table(struct node_s *n, bool recurse)
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
        node_free_table(n, recurse);
    else if(n->len < (n->max >> 2))
        node_resize_table(n, n->max >> 1);

    return n->max;
}

/*
 * Remove the node from its owner's table if an owner exists.
 * Assume that value stored at n->owner is a valid node pointer and
 * that the owner node remembers us (ie. that its table has a record of us).
 */
static void node_emancipate(struct node_s *n)
{
    /*
     * If we don't belong to anyone, there's nothing for us to do here.
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
    node_tighten_table(n->owner, true);

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
 * non-static functions
 */

/*
 * void node_free(struct node_s *n, bool recurse)
 * Free the current node, its value and recursively free
 * all of its children and their values and so on.
 */
void node_free(struct node_s *n, bool recurse)
{
    /*
     * Sanity check to make sure there is a node being passed in.
     */
    if(!n)
        return;
    pr_dbg("%s (%s) | recurse: %c", node_string(n), n->type->name, recurse ? 'T' : 'F');

    /*
     * After this, all the children along with the table itself
     * will have been freed.
     */
    if(recurse && n->table)
        node_free_table(n, recurse);

    /*
     * Remove ourselves from any owner nodes.
     */
    node_emancipate(n);

    /*
     * Check if it's our responsibility to free the data.
     */
    if(n->frees_data)
        /*
         * The 'freev' function provided by the type must handle freeing all
         * its data internals up to and including n->data itself as it sees fit.
         */
        n->type->freev(n->data);

    /*
     * Free the node itself.
     */
    node_free_all(n->str);
    n->str = 0;

    n->data = 0;
    free(n);
}

/*
 * struct node_s *node_new(const struct node_type_s *type, const void *d, bool fsd);
 * Create a new node given its type and a const representation of its data.
 */
struct node_s *node_new(const struct node_type_s *type, const void *d, bool fsd)
{
    pr_dbg("type: %s, d: %p", type->name, d);
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
    n->frees_data = fsd;
    n->owner = 0;
    n->table = 0;
    n->id = 0;
    n->len = 0;
    n->max = 0;
    n->count = 1;
    n->str = 0;
    node_to_str(n);

    pr_dbg("n: %s (%s)", node_string(n), n->type->name);
    return n;
}

/*
 * int node_diff(const struct node_s *a, const struct node_s *b);
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
 * struct node_s *node_to_str(struct node_s *n)
 * Returns the str representation of the node.
 */
struct node_s *node_to_str(struct node_s *n)
{
    if(!n)
        return 0;

    if(n->type == node_type_str)
            return n;

    if(n->type == node_type_node)
        return 0;

    node_free_all(n->str);

    n->str = n->type->to_str(n->data);
    return n->str;
}

/*
 * char *node_string(struct node_s *n);
 * Get the str buf associated with this node.
 */
char *node_string(struct node_s *n)
{
    if(!n)
        return 0;

    if(n->type == node_type_node)
        return node_string((struct node_s *) n->data);

    if(n->type == node_type_str)
        return str_node_buf(n);

    return str_node_buf(n->str);
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
    pr_dbg("n: %p, c: %p", n, c);

    /*
     * Sanitize. The node's table acts as a set, so we make sure
     * that we're not re-entering the same element twice.
     */
    if(!n || !c || (c->owner == n))
        return 0;
    /*
     * Clear away any previous child elements.
     */
    if(node_at(n, index))
        node_emancipate(node_at(n, index));

    /*
     * Make room for the new element as necessary.
     */
    if((index >= n->max) &&
        !node_resize_table(n, (index ? index : 1) << 1))
        return 0;

    if(index >= n->len) {
        /*
         * Clear all pointers leading up from the previously
         * set element.
         */
        node_clear_table(n, n->len, index - n->len);

        /*
         * Update the length only if the item we've inserted
         * is beyond the previous array length.
         */
        n->len = index + 1;
    }

    node_adopt(n, c, index);

    return n->len;
}

int node_bst_insert(struct node_s *a, struct node_s *b)
{
    if(!a || !b || (a->type != b->type))
        return 0;

    int diff = node_diff(a, b);

    if(diff < 0) {
        if(node_at(a, NODE_RIGHT)) {
            return node_bst_insert(node_at(a, NODE_RIGHT), b);
        } else {
            node_put(a, NODE_RIGHT, b);
        }
    } else {
        if(node_at(a, NODE_LEFT)) {
            return node_bst_insert(node_at(a, NODE_LEFT), b);
        } else {
            node_put(a, NODE_LEFT, b);
        }
    }

    return b->count;
}

void node_bt_for_each(struct node_s *n, void(*iter)(struct node_s *),
    enum node_order_e o)
{
    if(!n || !iter)
        return;

    if(o > NODE_PRE_ORDER) {
        node_bt_for_each(node_at(n, NODE_LEFT), iter, o);
        if(o == NODE_POST_ORDER)
            node_bt_for_each(node_at(n, NODE_RIGHT), iter, o);
    }

    iter(n);

    switch(o) {
        case NODE_PRE_ORDER:
            node_bt_for_each(node_at(n, NODE_LEFT), iter, o);
        case NODE_IN_ORDER:
            node_bt_for_each(node_at(n, NODE_RIGHT), iter, o);
        default:
            ;
    }
}

/*
 * struct node_s *node_release(struct node_s *n, size_t index)
 *  Release and return a child node from a parent.
 *
 * inputs:
 *  struct node_s *n - a pointer to the node containing the child
 *  element you want released size_t index - the index of the child
 *  node within the parent nodes table.
 *
 * output:
 *  struct node_s * - a pointer to the child node if one exists.
 *  Otherwise, return 0.
 *
 * notes:
 *  - This function shrinks the parent node's table as necessary.
 *  - If you have a node pointer c belonging to another node n you
 *      can look up c's index in n's table with 'c->id'.
 */
struct node_s *node_release(struct node_s *n, size_t index)
{
    /*
     * Look-up the data.
     */
    struct node_s *ret = node_at(n, index);

    /*
     * The 'node_at' macro has done bounds and sanity checks for us,
     * so all we have to do is make sure we've received something
     * back from it.
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
 * The node type
 */
struct node_type_s _type_node = {
    .size = sizeof(struct node_s),
    .freev = _node_free,
    .new = _node_new,
    .diff = _node_diff,
    .to_str = to_str,
    .name = "node"
};

const struct node_type_s *node_type_node = &_type_node;
