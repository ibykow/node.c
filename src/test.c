#include "common.h"
#include "test.h"

#define test_func(name) void test_##name (struct test_summary_s *res)
#define test_run(name) struct test_summary_s name##_tr = {0, 0}; \
    test_##name(&name##_tr); \
    test_summarize(&name##_tr, #name); \
    all_tr.passed += name##_tr.passed; \
    all_tr.failed += name##_tr.failed

struct test_summary_s {
    unsigned passed, failed;
};

static void test_summarize(struct test_summary_s *sum, const char *name)
{
    unsigned total = sum->passed + sum->failed;
    float passrate_f = (float) sum->passed / (float) total;
    long passrate = lroundf(passrate_f) * 100;
    printf("==%s test summary==\n", name);
    printf("pass/fail:total %u/%u:%u, rate %ld%%\n\n", sum->passed, sum->failed,
        total, passrate);
}

static struct node_s *random_str_graph(unsigned num_verts, const char *name)
{
    struct node_s *g = node_new_node(str_node_new(name));
    if(!g)
        return 0;

    pr_dbg("g: %p", g);

    unsigned i = 0;
    for(i = 0; i < num_verts; i++)
    {
        char s[4];
        s[0] = 'a' + (i % 26),
        s[1] = 'b' + (i % 26),
        s[2] = 'c' + (i % 26);
        s[3] = 0;

        node_put(g, i, str_node_new(s));
        pr_dbg("g->table: %p, g->table[%u]: %p", g->table, i, g->table[i]);
    }

    return g;
}

test_func(table)
{
    size_t i;
    struct node_s *t = str_node_new("Table Test Node");
    test_fail(res, !t, "couldn't create test node");
    // Add 100 items in a row
    for(i = 0; i < 100; i++) {
        node_push(t, str_node_new("table test item"));
        test_break(res, t->len != (i + 1),
            "insertion. Table len is %lu. Should be %lu\n",
                    t->len, i + 1);
    }
    // Remove 100 items
    struct node_s *next;
    while((next = node_pop(t)))
        node_free(next, true);

    test_try(res, t->len, "couldn't remove item %lu", t->len);

    // Check that the table is empty (table pointer, len, max)
    test_try(res, t->table, "table pointer still exists");
    test_try(res, t->max, "table max is %lu. Should be 0", t->max);
}

test_func(stack)
{
    size_t i;
    struct node_s *stack = 0, *next;

    for(i = 0; i < TEST_ROUNDS; i++) {
        stack_push(&stack, str_node_new("bbb"));
        test_try(res, !stack, "couldn't push item %lu onto the stack", i);
    }

    for(i = 0; (next = stack_pop(&stack)); i++) {
        test_try(res, !next, "couldn't pop item %lu", i);
        node_free(next, true);
    }

    test_try(res, stack, "stack shouldn't exist");
}

test_func(queue)
{
    size_t i;
    struct node_s *q = 0, *next;

    for(i = 0; i < TEST_ROUNDS; i++) {
        q_en(&q, str_node_new("qqq"));
        test_try(res, !q, "couldn't enqueue %lu item", i);
    }

    for(i = 0; (next = q_de(&q)); i++) {
        test_try(res, !next, "couldn't dequeue item %lu", i);
        node_free(next, true);
    }

    test_try(res, q, "queue shouldn't exist");
}

test_func(list)
{
    size_t i;
    struct node_s *head = str_node_new("aaa"), *next = head;
    test_fail(res, !head, "couldn't create head node");

    for(i = 0; i < TEST_ROUNDS; i++) {
        node_push(next, str_node_new("aaa"));
        next = node_at(next, 0);
        test_break(res, !next, "couldn't add item %lu", i);
    }

    for(i = 0, next = head; (next = node_at(next, 0)); i++) {
        test_break(res, !next, "couldn't remove item %lu", i);
        test_try(res, !next->owner, "owner %lu was not set", i);
        test_try(res, next->type->diff(next->data, next->owner->data),
            "diff %lu not working", i);
    }

    node_free(head, true);
}

test_func(str_graph)
{
    struct node_s *g = random_str_graph(10, "My graph");
    test_fail(res, !g, "couldn't create string graph");

    // unsigned j;
    // printf("%s %lu\n", str_node_buf(node_data(g)), g->len);
    // for(j = 0; j < g->len; j++)
    //     printf("%u %s\n", j, str_node_buf(node_at(g, j)));

    node_free(g, true);
}

test_func(basic)
{
    size_t i;
    for(i = 1; i <= TEST_ROUNDS; i++) {
        // printf("Test round %u\n", i);
        struct node_s   *sn1 = str_node_new("Hello"),
                        *sn2 = str_node_new("World"),
                        *n1 = node_new_node(sn1);

        pr_dbg("sn1: %p, n1: %p %p", sn1, n1, node_type_node);

        test_fail(res, !sn1, "couldn't create first node");
        test_fail(res, !sn2, "couldn't create second node");
        test_try(res, !n1, "couldn't create nodeception (node within a node)");
        test_try(res, !node_put(sn1, 1, sn2), "couldn't insert node into set");
        test_try(res, sn1->len != 2, "node table has wrong length");
        test_try(res, !node_put(n1, 1, sn2), "couldn't move node into another set");
        test_try(res, !node_release(n1, 1), "couldn't remove node");
        test_try(res, !!sn1->table, "table was not freed");
        test_try(res, !node_push(sn1, sn2), "couldn't add to list");
        test_try(res, node_put(sn1, 0, sn2), "inserted same item into set");
        test_try(res, node_put(sn1, 10, sn2), "inserted same item into set");
        // printf("%s\n", str_node_buf(sn1));
        node_free(n1, true);
    }
}

int main(int argc, char const *argv[])
{
    pr_dbg("start");
    struct test_summary_s all_tr = { 0, 0 };
    unsigned i;
    for(i = 0; i < TEST_ROUNDS; i++) {
        test_run(basic);
        test_run(list);
        test_run(stack);
        test_run(queue);
        test_run(table);
    }

    test_summarize(&all_tr, "all");


    return 0;
}
