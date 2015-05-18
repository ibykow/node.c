#include "common.h"
#include "test.h"

int main(int argc, char const *argv[])
{
    unsigned i;
    struct node_s *head = str_node_new("aaa"), *next = head;
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
        node_free(sn1);
        node_free(n1);

        node_add(next, str_node_new("aaa"));
        fail(!next->len, "couldn't add to linked list");
        next = next->table[0];
    }

    for(i = 0, next = head; (next = next->table ? next->table[0] : 0); i++) {
        fail(!next->owner, "owner was not set");
        fail(next->type->diff(next->data, next->owner->data), "diff not working");
    }

    fail(i != TEST_ROUNDS, "linked list is too short");
    node_free(head);

    printf("All test rounds passed!\n");
    return 0;
}
