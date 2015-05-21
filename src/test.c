// #define PR_DEBUG
#include "common.h"
#include "test.h"

// test_summarize(&name##_tr); \//
#define test_func(name) static void test_##name (struct test_result_s *res)
#define test_run(name) struct test_result_s name##_tr = test_result_new(#name); \
    test_##name(&name##_tr); \
    test_pass_round(name##_tr, #name); \
    global_tr.passed += name##_tr.passed; \
    global_tr.failed += name##_tr.failed

static void test_summarize(struct test_result_s *sum)
{
    unsigned total = sum->passed + sum->failed;
    sum->rate = ((float) sum->passed / (float) total) * 100;
    printf("\n%s test results:\n", sum->name);
    printf("%u tests. %u passed. %u failed. %.02f%% pass rate.\n\n",
        total, sum->passed, sum->failed, sum->rate);
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

test_func(basic)
{
    size_t i;
    for(i = 1; i <= TEST_ROUNDS; i++) {
        // printf("Test round %u\n", i);
        struct node_s   *sn1 = str_node_new("Hello"),
                        *sn2 = str_node_new("World"),
                        *n1 = node_new_node(sn1);

        pr_dbg("sn1: %p, n1: %p %p", sn1, n1, node_type_node);

        test_fail(!sn1, "couldn't create first node");
        test_fail(!sn2, "couldn't create second node");
        test_try(!n1, "couldn't create nodeception (node within a node)");
        test_try(!node_put(sn1, 1, sn2), "couldn't insert node into set");
        test_try(sn1->len != 2, "node table has wrong length");
        test_try(!node_put(n1, 1, sn2), "couldn't move node into another set");
        test_try(!node_release(n1, 1), "couldn't remove node");
        test_try(!!sn1->table, "table was not freed");
        test_try(!node_push(sn1, sn2), "couldn't add to list");
        test_try(node_put(sn1, 0, sn2), "inserted same item into set");
        test_try(node_put(sn1, 10, sn2), "inserted same item into set");

        // printf("%s\n", str_node_buf(sn1));
        node_free_all(n1);
    }
}

test_func(table)
{
    size_t i;
    struct node_s *t = str_node_new("Table Test Node");
    test_fail(!t, "couldn't create test node");
    // Add 100 items in a row
    for(i = 0; i < 100; i++) {
        node_push(t, str_node_new("table test item"));
        test_break(t->len != (i + 1),
            "insertion. Table len is %lu. Should be %lu\n",
                    t->len, i + 1);
    }
    // Remove 100 items
    struct node_s *next;
    while((next = node_pop(t)))
        node_free_all(next);

    test_try(t->len, "couldn't remove item %lu", t->len);

    // Check that the table is empty (table pointer, len, max)
    test_try(t->table, "table pointer still exists");
    test_try(t->max, "table max is %lu. Should be 0", t->max);
    node_free_all(t);
}

test_func(list)
{
    size_t i;
    struct node_s *head = str_node_new("aaa"), *next = head;
    test_fail(!head, "couldn't create head node");

    for(i = 0; i < TEST_ROUNDS; i++) {
        node_push(next, str_node_new("aaa"));
        next = node_at(next, 0);
        test_break(!next, "couldn't add item %lu", i);
    }

    for(i = 0, next = head; (next = node_at(next, 0)); i++) {
        test_break(!next, "couldn't remove item %lu", i);
        test_try(!next->owner, "owner %lu was not set", i);
        test_try(next->type->diff(next->data, next->owner->data),
            "diff %lu not working", i);
    }

    node_free_all(head);
}

test_func(stack)
{
    size_t i;
    struct node_s *stack = 0, *next;

    for(i = 0; i < TEST_ROUNDS; i++) {
        stack_push(&stack, str_node_new("bbb"));
        test_try(!stack, "couldn't push item %lu onto the stack", i);
    }

    for(i = 0; (next = stack_pop(&stack)); i++) {
        test_try(!next, "couldn't pop item %lu", i);
        node_free_all(next);
    }

    test_try(stack, "stack shouldn't exist");
}

test_func(queue)
{
    size_t i;
    struct node_s *q = 0, *next;

    for(i = 0; i < TEST_ROUNDS; i++) {
        q_en(&q, str_node_new("qqq"));
        test_try(!q, "couldn't enqueue item %lu", i);
    }

    for(i = 0; (next = q_de(&q)); i++) {
        test_try(!next, "couldn't dequeue item %lu", i);
        node_free_all(next);
    }

    test_try(i != TEST_ROUNDS, "expected %d items. Had %lu", TEST_ROUNDS, i);

    test_try(q, "queue shouldn't exist");
}

test_func(graph)
{
    struct node_s *g = random_str_graph(10, "My graph");
    test_fail(!g, "couldn't create string graph");

    // unsigned j;
    // printf("%s %lu\n", str_node_buf(node_data(g)), g->len);
    // for(j = 0; j < g->len; j++)
    //     printf("%u %s\n", j, str_node_buf(node_at(g, j)));

    node_free_all(g);
}

static int prev_int;
static bool fail_flag;

static void confirm_ascended(struct node_s *n)
{
    if(int_node_n(n) < prev_int)
        fail_flag = true;

    pr_dbg("%d %s %d", int_node_n(n), int_node_n(n) < prev_int ? "<" : ">=", prev_int);
    prev_int = int_node_n(n);
}

test_func(btree)
{
    const unsigned num_nodes = 200;
    struct node_s *t = int_node_new(num_nodes >> 1);
    test_fail(!t, "couldn't create root node");
    pr_dbg("Created root node %d", int_node_n(t));

    unsigned i;

    for(i = 0; i < num_nodes; i++) {
        struct node_s *n = int_node_new(ur(num_nodes));
        test_try(!n, "could create node %u", i);
        test_try(!node_bst_insert(t, n), "couldn't insert node %u (%d)",
            i, int_node_n(n));

        pr_dbg("Added node %u: %d", i, int_node_n(n));
    }

    prev_int = -1; fail_flag = false;
    node_in_order(t, confirm_ascended);
    test_try(fail_flag, "btree is out of order");

    node_free_all(t);
}

int main(int argc, char const *argv[])
{
    init_random();
    struct test_result_s global_tr = test_result_new("global");
    unsigned i;

    printf("Running %s tests\n", global_tr.name);
    for(i = 0; i < TEST_ROUNDS; i++) {
        test_run(basic);
        test_run(list);
        test_run(stack);
        test_run(queue);
        test_run(graph);
        test_run(table);
        test_run(btree);
    }

    test_summarize(&global_tr);

    return 0;
}
