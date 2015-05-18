#include "common.h"

struct str_s *_str_new(const char *s, unsigned len)
{
    if(!len)
        return 0;

    struct str_s *new = (struct str_s *) malloc(sizeof(struct str_s));
    if(!new)
        return 0;

    new->buf = (char *) malloc(sizeof(char) * len + 1);
    if(!new->buf) {
        free(new);
        return 0;
    }

    snprintf(new->buf, len + 1, "%s", s);
    new->len = len;

    return new;
}

static void str_free(void *data)
{
    if(!data)
        return;

    struct str_s *str = str_get(data);

    if(!str->len)
        return;

    free(str->buf);
    free(data);
}

static void *str_new(const void *init)
{
    if(!init)
        return 0;

    return (void *) _str_new(str_buf(init), strlen(str_buf(init)));
}

static int str_diff(const void *a, const void *b)
{
    if(a == b)
        return 0;

    if(!a || !b)
        return -1;

    if(str_len(a) != str_len(b))
        return -1;

    return strncmp(str_buf(a), str_buf(b), str_len(a));
}

static const struct node_type_s _type_str = {
    .size = sizeof(struct str_s),
    .freev = str_free,
    .new = str_new,
    .diff = str_diff
};

const struct node_type_s *node_type_str = &_type_str;
