#pragma once
#include <stdbool.h>
#include "../mempool.h"
#include "../unit_tests.h"


typedef struct stack stack;

stack *new_stack(mempool *mp);
int   stack_length(stack *s);
bool  stack_is_empty(stack *s);
void  stack_clear(stack *s);
void  stack_push(stack *s, void *item);
void *stack_peek(stack *s);
void *stack_pop(stack *s);

#ifdef INCLUDE_UNIT_TESTS
void stack_unit_tests();
#endif