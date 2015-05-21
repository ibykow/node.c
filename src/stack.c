/*
 * stack.c
 *
 * A simple node-based stack implementation.
 *
 * Ilia Bykow, 2015
 */

#include "common.h"

#define STACK_INDEX 0

/*
 * void stack_push(struct node_s **stack, const struct node_s *n);
 * Push a node onto the stack.
 *
 * The **stack variable is simply a pointer to the address of a node
 * you've created for the purposes of being used as a stack. ie:
 *
 * struct node_s *stack = 0; // &stack can now be passed to stack_push.
 * When you define the pointer, make sure you set it to 0 initially,
 * that way the stack_push function can begin pushing elements onto it.
 *
 * Example usage of the push/pop mechanism:
 *
 * void stack_example(struct node_s my_nodes[], unsigned num_nodes)
 * {
 *     struct node_s *my_stack = 0, *item;
 *
 *      unsigned i;
 *      for(i = 0; i < num_nodes; i++) {
 *          // Push elements onto the stack.
 *          stack_push(&my_stack, some_node);
 *      }
 *
 *      // Pop elements off of the stack.
 *      while((item = stack_pop(&my_stack))) {
 *
 *          // do something with each node item here.
 *          node_free_all(item);
 *      }
 * }
 *
 * This function warps whatever node you want to push inside an outer
 * nested node structure. This way we avoid modifying the stack element's
 * table contents in any way.
 *
 * If you want to create a stack using your current node's table directly,
 * you should not use this function but rather, just put your node inside
 * the table of another node as in:
 *
 * node_put(my_stack_node, my_stack_index, my_node_to_push);
 *
 * where 'my_stack_node' is the node you want to use as your stack,
 * 'my_stack_index' is the table index which acts as your 'next' slot, and
 * 'my_node_to_push' is the node you want pushed to the top of your stack.
 */
void stack_push(struct node_s **stack, const struct node_s *n)
{
    /*
     * Sanitize.
     */
    if(!stack || !n)
        return;

    /*
     * Create a nested stack node around the node we want
     * to insert.
     */
    struct node_s *next = node_new_node_const(n);
    pr_dbg("*stack: %p, n: %p, next: %p", *stack, n, next);

    /*
     * Make sure the allocation went smoothly.
     */
    if(!next)
        return;

    /*
     * If the stack isn't empty, push it down.
     */
    if(*stack) {

        /*
         * Maintain the circular linked list.
         */
        if((*stack)->owner) {

            /*
             * The stack owner is the 'last' element on the stack.
             */
            node_put((*stack)->owner, STACK_INDEX, next);

        } else {

            /*
             * No owner means we haven't created a circular list yet.
             */
            node_put(*stack, STACK_INDEX, next);
        }

        node_put(next, STACK_INDEX, *stack);
    }

    /*
     * Move the stack up to the current head.
     */
    *stack = next;

}

/*
 * struct node_s *stack_pop(struct node_s **stack);
 * Return the top-most node from a stack.
 */
struct node_s *stack_pop(struct node_s **stack)
{

    /*
     * Sanitize.
     */
    if(!stack || !*stack)
        return 0;

    /*
     * Take out the next stack node as well as the data
     * from the current one.
     */
    struct node_s *next = node_release(*stack, STACK_INDEX),
                    *n = node_data(*stack);

    /*
     * Free the, now empty, head of the stack.
     */
    node_free_all(*stack);

    /*
     * Move the stack down to the new head.
     */
    *stack = next;

    /*
     * Return the data;
     */
    return n;
}

/*
 * struct node_s *stack_deq(struct node_s **stack);
 * Return the bottom-most node from the "stack" (queue).
 *
 * Since the stack is built as a doubly-linked circular list
 * We simply return the value stored in the stack's owner node.
 */
struct node_s *stack_deq(struct node_s **q)
{
    if(!q || !*q)
        return 0;

    struct node_s *n;
    pr_dbg("*q: %p, (*q)->type: %p", *q, (*q)->type);

    /*
     * Having an owner means that we have two or more items left in the queue.
     */
    if((*q)->owner) {

        /*
         * Remember the element before last.
         */
        struct node_s *prev = (*q)->owner->owner, *current;
        /*
         * Remember the node we want to get the data from and release it
         * from its owner.
         */
        current = node_release(prev, STACK_INDEX);

        /*
         * Check whether we're down to our last two elements.
         */
        if(*q == prev) {

            /*
             * If so, release the remaining element.
             */
            node_release(current, STACK_INDEX);

        } else {

            /*
             * Otherwise, reattach the new 'bottom' node with the current 'top'.
             */
            node_put(prev, STACK_INDEX, *q);
        }

        /*
         * Get the data we want to return.
         */
        n = node_data(current);
        pr_dbg("*q: %p, current: %p, prev: %p, n: %p, current->owner: %p, (*q)->owner: %p",
            *q, current, prev, n, current->owner, prev->owner);

        /*
         * Free the, now unused, empty and emancipated node.
         * We are assuming this node was created in the q_push process
         * and will therefore not free its data which is itself a node.
         */
        node_free_all(current);

    /*
     * Otherwise, if we don't have an owner then we are the last node
     * in the queue.
     */
    } else {

        /*
         * Hold on to the data.
         */
        n = node_data(*q);
        pr_dbg("*q: %p, n: %p - END OF QUEUE.............!", *q, n);

        /*
         * Free the last node.
         */
        node_free_all(*q);

        /*
         * Keep track of the fact that we're empty.
         */
        *q = 0;
    }

    return n;
}
