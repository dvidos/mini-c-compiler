#pragma once

struct array_vtable;

// an array holding references to other objects
// add() works by adding a pointer to the pointed item
typedef struct array {
    char *buffer;
    int length;
    int capacity;

    struct array_vtable *v;
} array;

array *new_array();

struct array_vtable {
    // simple array ops
    void (*clear)(array *a);
    void *(*get)(array *a, int index);
    void (*add)(array *a, void *item);
    void (*set)(array *a, int index, void *item);
    int (*index_of)(array *a, void *item);
    void (*insert_at)(array *a, int index, void *item);
    void (*remove_at)(array *a, int index);

    // queue, stack implementations
    void (*prepend)(array *a, void *item); // add from front
    void *(*dequeue)(array *a); // remove from front
    void (*push)(array *a, void *item); // same as add()
    void *(*pop)(array *a); // remove from end (last added)
    void *(*peek)(array *a);  // return last, without removing

    // involved operations
    void (*for_each)(array *a, visitor_func *visitor);
    array *(*find)(array *a, void *criteria, matcher_func *matcher);
    void (*append_all)(array *a, array *other);
    void (*sort)(array *a, comparer_func *comparer);
    int (*bin_search)(array *a, void *criteria, comparer_func *comparer);
    array *(*filter)(array *a, filter_func *filter);
    array *(*map)(array *a, mapper_func *mapper);
    void *(*reduce)(array *a, void *init_value, reducer_func *reduce);
    void (*deduplicate)(array *a, comparer_func *comparer)
    str *(*join)(array *a, str *separator, to_str_func *to_str);
    long (*hash)(array *a, hasher_func *hasher);

    void (*free)(array *a, visitor_func *free_item); 
};
