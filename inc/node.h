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

#define nested_node_new(n, freeit) node_new(nested_node_type, \
    &(struct nested_node_s) {n, freeit})

/*
 * Helper macros for accessing nested node members.
 */
#define nested_node_get(d) ((struct nested_node_s *) d)
#define nested_node_member(d, member) nested_node_get(d)->member
#define nested_node_node(d) nested_node_member(d, node)
#define nested_node_freeit(d) nested_node_member(d, freeit)


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
 * The basic node data structure.
 */
struct node_s {
    void *data;
    const struct node_type_s *type;
    struct node_s *owner, **table;
    size_t id, len, max;
};

/*
 * The data structure representing nested nodes.
 * All nested nodes must be provided with a previously created node and
 * told whether you want those nodes freed along with the nested node.
 * This way we can avoid constantly making copies.
 */
struct nested_node_s {
    struct node_s *node;
    unsigned freeit;
};

/*
 * Used for creating nodes which have other nodes as their "data".
 */
extern const struct node_type_s *nested_node_type;

/*
 * functions
 */
void node_free(struct node_s *);
struct node_s *node_new(const struct node_type_s *type, const void *d);
int node_diff(const struct node_s *a, const struct node_s *b);
size_t node_put(struct node_s *, size_t, struct node_s *);
struct node_s *node_release(struct node_s *, size_t);

#endif
