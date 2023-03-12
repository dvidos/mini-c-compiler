#pragma once


struct iterator_vtable;

typedef struct iterator {
    arr *arr;
    int current;

    struct iterator_vtable *v;
} iterator;

iterator *new_iterator(arr *a);

struct iterator_vtable {
    void (*reset)(iterator *i); // go before the first element
    void *(*curr)(iterator *i); // returns current, does not advance
    bool (*has_next)(iterator *i); 
    void *(*next)(iterator *i); // null if none, 
};

