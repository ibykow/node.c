#include "common.h"

static void int_free(void *data)
{
    if(!data)
        return;

    free(data);
}

static void *int_new(const void *init)
{
    struct int_s *new = (struct int_s *) malloc(sizeof(struct int_s));
    if(!new)
        return 0;

    new->n = *(int *) init;
    return (void *) new;
}

static int int_diff(const void *a, const void *b)
{
    return int_get_n(a) - int_get_n(b);
}

static struct node_s *int_to_str(const void *d)
{
    char s[100];
    snprintf(s, 100, "%d", int_get_n(d));
    return str_node_new(s);
}

static const struct node_type_s _type_int = {
    .size = sizeof(struct int_s),
    .freev = int_free,
    .new = int_new,
    .diff = int_diff,
    .to_str = int_to_str,
    .name = "integer"
};

const struct node_type_s *node_type_int = &_type_int;
