#ifndef _NODE_H_
#define _NODE_H_

#include <stdlib.h>

#define node_add(n, new) (n ? node_put(n, n->len, new) : 0)
#define node_at(n, i) (n && (n->len > index) ? n->table[i] : 0)

struct node_type_s {
    size_t size;
    void (*freev)(void *),
        *(*new)(const void *);
    int (*diff)(const void *, const void *);
};

struct node_s {
    void *data;
    const struct node_type_s *type;
    struct node_s *owner, **table;
    size_t id, len, max;
};

void node_free(struct node_s *);
struct node_s *node_new(const struct node_type_s *, const void *,
    struct node_s *, size_t);

size_t node_put(struct node_s *, size_t, struct node_s *);
struct node_s *node_release(struct node_s *, size_t);

#endif
