#ifndef _NODE_H_
#define _NODE_H_

/*
 * node.h
 *
 * Create generic, associative data types and structures.
 * See here and node.c for comments and descriptions.
 */

#include <stdlib.h>

/*
 * macros
 */

#define NODE_TYPE_DIFF (-1)

#define node_node_new(n) node_new(node_type, n)

/*
 * Helper macros for accessing node members.
 */
#define node_get(d) ((struct node_s *) d)
#define node_member(d, member) node_get(d)->member
#define node_data(d) node_member(d, data)
#define node_get_type(d) node_member(d, type)
#define node_get_diff(d) node_member(d, type)->diff

/*
 * Basic, shallow copy. This will recrusively copy and nodes
 * which have other nodes as their data.
 */
#define node_copy(n) (n ? node_new(n->type, n->data) : 0)

/*
 * Add a child node to the end of the parent's table.
 */
#define node_add(n, new) (n ? node_put(n, n->len, new) : 0)

/*
 * Check what's at index i in n's table.
 */
#define node_at(n, i) (n && (n->len > index) ? n->table[i] : 0)

/*
 * data structures
 */

/*
 * To create a new node type, make a variable with the fields filled in,
 * then create an extern const pointer to it. You can then use that pointer
 * as the *type* required by node_new.
 *
 * See str.c and str.h for an example of how this is done. Also, we declared
 * our own node type further down to be used to store nodes within nodes.
 */
struct node_type_s {
    size_t size;
    void (*freev)(void *),
        *(*new)(const void *);
    int (*diff)(const void *, const void *);
};

/*
 * This is the basic node data structure. Note that the data pointer is
 * situated at the top of the structure for immediate access if it is to
 * be nested in another structure.
 */
struct node_s {
    void *data;
    const struct node_type_s *type;
    struct node_s *owner, **table;
    size_t id, len, max;
};

/*
 * The const node_type_s declaration used for creating nodes which have
 * other nodes as their "data".
 */
const struct node_type_s *node_type;

/*
 * functions
 */
void node_free(struct node_s *);
struct node_s *node_new(const struct node_type_s *type, const void *d);
size_t node_put(struct node_s *, size_t, struct node_s *);
struct node_s *node_release(struct node_s *, size_t);

#endif
