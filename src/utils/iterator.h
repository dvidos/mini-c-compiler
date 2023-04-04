#pragma once
#include "func_types.h"

// maybe this helps? -- maybe pair with strongly typed array?
#define strongly_typed_iterator(iter, T) \
    inline T *next_#T(iter) { return (T*)(iter->v->next(iter)); } \
    inline T *curr_#T(iter) { return (T*)(iter->v->curr(iter)); } \

#define for_typed_iteration(iter, T, ptr) \
    for (T *ptr = (T *)iter->reset(); iter->valid(); ptr = (T *)iter->next())

struct iterator_vtable;

typedef struct iterator {
    arr *arr;
    int current;

    struct iterator_vtable *v;
} iterator;

iterator *new_iterator(arr *a, int start_idx, int upper_limit, bool descending, filter_func *filter);

struct iterator_vtable {
    void (*reset)(iterator *i); // go before the first element
    void *(*curr)(iterator *i); // returns current, does not advance
    bool (*has_next)(iterator *i); 
    void *(*next)(iterator *i); // null if none, 
};

