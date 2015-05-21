#ifndef STR_H_
#define STR_H_

#define str_get(d) ((struct str_s *) (d))
#define str_at(d, member) str_get(d)->member
#define str_len(d) str_at(d, len)
#define str_buf(d) str_at(d, buf)

#define str_node_get(n) str_get(n->data)
#define str_node_at(n) str_at(n->data)
#define str_node_len(n) str_len(n->data)
#define str_node_buf(n) (n ? str_buf(n->data) : "no node!")

#define str_init(s, l) &(const struct str_s) {.buf = (char *) s, .len = l}

#define str_node_new(s) node_new(node_type_str, str_init(s, strlen(s)), true)

#define str_node_new_const(s) \
    node_new(node_type_str, str_init(s, strlen(s)), false)

struct str_s {
    char *buf;
    size_t len;
};

extern const struct node_type_s *node_type_str;

#endif
