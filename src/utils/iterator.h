#pragma once
#include "func_types.h"

struct iterator_vtable;

typedef struct iterator {
    void *priv_data;
    struct iterator_vtable *v;
} iterator;

struct iterator_vtable {
    // reset and return the first item, if applicable
    void *(*reset)(iterator *i);

    // whether the last returned item (reset() or next()) was valid
    bool (*valid)(iterator *i);

    // advance and return the next item, if applicable
    void *(*next)(iterator *i);

    // free data and object
    void (*free)(iterator *i);
};


// helpful function for primitive arrays
// "array" and "hashtable" generate their own iterators
iterator *new_array_iterator(void *arr[], int start_idx, int upper_limit, bool descending, filter_func *filter);


// macro to aid in syntax
#define foreach(T, ptr, iter)   for (T *ptr = (T *)iter->reset(); iter->valid(); ptr = (T *)iter->next())


// -----------------------------------------------

// used in a "while" format
void *ptr = it->v->reset(it);
while (it->v->valid(it)) {
    ..
    ptr = it->v->next(it):
}
// used in in a "for" format
for (void *ptr = it->v->reset(it); it->v->valid(it); ptr = it->v->next(it)) {
    ...
}
