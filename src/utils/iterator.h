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
    array *arr;
    int current;

    struct iterator_vtable *v;
} iterator;

iterator *new_iterator(array *a, int start_idx, int upper_limit, bool descending, filter_func *filter);

struct iterator_vtable {
    void (*reset)(iterator *i); // go before the first element
    void *(*curr)(iterator *i); // returns current, does not advance
    bool (*valid)(iterator *i); // whether current is valid (through reset() or next())
    void *(*next)(iterator *i); // null if none, 
};

iterator *it;

// using a while format
it->v->reset();
while (it->v->valid()) {
    void *ptr = it->v->curr();

    it->v->next():
}

// using a for format
for (it->v->reset(), ptr = it->v->curr(); it->v->valid(); it->v->next(), ptr = it->v->curr()) {

}



