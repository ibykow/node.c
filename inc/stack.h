#ifndef STACK_H_
#define STACK_H_

/*
 * stack.h
 *
 * Pushing onto the top of the stack is the exact same action
 * we take when enqueuing an item. Only the dequeue process differs
 * from its stack counterpart. Popping returns the item from the top
 * of the stack while dequeuing returns the item from the bottom.
 * Hence stack_enq and stack_push are synonymous while stack_pop and
 * stack_deq are implemented as separate functions.
 *
 * We may further consider developing an "unshift" operation which
 * would place an item back onto the bottom of the stack. This would
 * provide us with "four-way" add/remove functionality.
 *
 * For further comments see stack.c
 */
#define stack_enq stack_push

#define q_en stack_enq
#define q_de stack_deq

void stack_push(struct node_s **, const struct node_s *);
struct node_s *stack_pop(struct node_s **);
struct node_s *stack_deq(struct node_s **);

#endif
