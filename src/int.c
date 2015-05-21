#include "common.h"

static void int_free(void *data)
{
    if(!data)
        return;

    free(data);
}

static void *int_new(const void *init)
{
    if(!init)
        return 0;

    struct int_s *new = (struct int_s *) malloc(sizeof(struct int_s));
    if(!new)
        return 0;

    new->n = *(int *) init;

    return (void *) new;
}

static int int_diff(const void *a, const void *b)
{
    return a - b;
}

static const struct node_type_s _type_int = {
    .size = sizeof(struct int_s),
    .freev = int_free,
    .new = int_new,
    .diff = int_diff
};

const struct node_type_s *node_type_int = &_type_int;
