#ifndef INT_H_
#define INT_H_

#define int_get(d) ((struct int_s *) d)
#define int_get_n(d) int_get(d)->n 
#define int_node_get(n) int_get(n->data)
#define int_node_n(node) int_get(((struct node_s *) node)->data)->n

#define int_init(n) &(const struct int_s) {n}

#define int_node_new(n) node_new(node_type_int, int_init(n), true)
#define int_node_new_const(n) node_new(node_type_int, int_init(n), false)

struct int_s { int n; };

extern const struct node_type_s *node_type_int;

#endif
