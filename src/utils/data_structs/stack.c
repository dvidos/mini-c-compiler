#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "stack.h"


typedef struct stack_node {
    void *item;
    struct stack_node *next;
} stack_node;

struct stack {
    int items_count;
    stack_node *top; // where we push and pop
    mempool *mempool;
};

stack *new_stack(mempool *mp) {
    stack *s = mpalloc(mp, stack);
    s->mempool = mp;
    return s;
}

int stack_length(stack *s) {
    return s->items_count;
}

bool stack_is_empty(stack *s) {
    return s->items_count == 0;
}

void stack_clear(stack *s) {
    s->top = NULL;
    s->items_count = 0;
}

void stack_push(stack *s, void *item) {
    stack_node *n = mpalloc(s->mempool, stack_node);
    n->item = item;
    n->next = NULL;

    if (s->top == NULL) {
        s->top = n;
        s->items_count = 1;
    } else {
        n->next = s->top;
        s->top = n;
        s->items_count++;
    }
}

void *stack_peek(stack *s) {
    return s->top == NULL ? NULL : s->top->item;
}

void *stack_pop(stack *s) {
    if (s->top == NULL)
        return NULL;

    stack_node *n = s->top;  // what a joy to not have to free() !!!
    s->top = s->top->next;
    s->items_count--;

    return n->item; 
}

#ifdef INCLUDE_UNIT_TESTS
void stack_unit_tests() {
    mempool *mp = new_mempool();

    // create
    stack *s = new_stack(mp);
    assert(s != NULL);

    // assert empty, length, get returns NULL
    assert(stack_is_empty(s));
    assert(stack_length(s) == 0);
    assert(stack_peek(s) == NULL);

    // push two things
    char c1 = 'a';
    char c2 = 'b';
    stack_push(s, &c1);
    stack_push(s, &c2);

    // assert non empty, length
    assert(!stack_is_empty(s));

    // assert peek, then get one, peek, then get the other
    assert(stack_length(s) == 2);
    assert(stack_peek(s) == &c2);
    assert(stack_pop(s) == &c2);

    assert(stack_length(s) == 1);
    assert(stack_peek(s) == &c1);
    assert(stack_pop(s) == &c1);
    
    assert(stack_length(s) == 0);
    assert(stack_peek(s) == NULL);
    assert(stack_is_empty(s));

    // add, clear
    stack_push(s, &c1);
    assert(stack_length(s) == 1);
    stack_clear(s);
    assert(stack_length(s) == 0);

    mempool_release(mp);
}
#endif

