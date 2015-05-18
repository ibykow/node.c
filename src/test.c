#include "common.h"
#include "test.h"

int main(int argc, char const *argv[])
{
    pr_dbg("start");
    unsigned i;
    struct node_s   *head = str_node_new("aaa"), *next = head,
                    *stack = 0, *q = 0;
    fail(!head, "couldn't create head node");

    for(i = 1; i <= TEST_ROUNDS; i++) {
        struct node_s   *sn1 = str_node_new("Hello"),
                        *sn2 = str_node_new("World"),
                        *n1 = nested_node_new(sn1, 0);

        fail(!sn1, "couldn't create first node");
        fail(!sn2, "couldn't create second node");
        fail(!n1, "couldn't create nodeception (node within a node)");
        fail(!node_put(sn1, 1, sn2), "couldn't insert node into set");
        fail(sn1->len != 2, "node table has wrong length");
        fail(!node_put(n1, 1, sn2), "couldn't move node into another set");
        fail(!node_release(n1, 1), "couldn't remove node");
        fail(!!sn1->table, "table was not freed");
        fail(!node_add(sn1, sn2), "couldn't add to list");
        fail(node_put(sn1, 0, sn2), "inserted same item into set");
        fail(node_put(sn1, 10, sn2), "inserted same item into set");
        // printf("%s\n", str_node_buf(sn1));
        node_free(sn1, true);
        node_free(n1, true);

        node_add(next, str_node_new("aaa"));
        fail(!next->len, "couldn't add to linked list");
        // next = next->table[0];
        next = node_at(next, 0);

        stack_push(&stack, str_node_new("bbb"));
        fail(!stack, "couldn't push item onto the stack");

        q_en(&q, str_node_new("qqq"));
        fail(!is_nested_node(q), "couldn't enqueue an item");
    }

    for(i = 0, next = head; (next = next->table ? next->table[0] : 0); i++) {
        fail(!next->owner, "owner was not set");
        fail(next->type->diff(next->data, next->owner->data), "diff not working");
    }

    fail(i != TEST_ROUNDS, "linked list is too short");
    node_free(head, true);

    for(i = 0; (next = stack_pop(&stack)); i++)
        node_free(next, true);
    fail(i != TEST_ROUNDS, "stack was too short");

    for(i = 0; (next = q_de(&q)); i++) {
        // printf("%s\n", str_node_buf(next));
        node_free(next, false);
    }
    fail(i != TEST_ROUNDS, "queue was too short");

    printf("All test rounds passed!\n");
    return 0;
}
