#pragma once
#include "string.h"
#include "iterator.h"

struct list_vtable;

// an list holding pointers to other objects
typedef struct list {
    void *priv_data;
    struct list_vtable *v;
} list;


list *new_list();

struct list_vtable {

    // simple list ops
    void (*clear)(list *l);
    int (*length)(list *l);
    void *(*get)(list *l, int index);
    void (*add)(list *l, void *item);
    void (*set)(list *l, int index, void *item);
    int (*index_of)(list *l, void *item);
    void (*insert_at)(list *l, int index, void *item);
    void (*remove_at)(list *l, int index);

    // queue, stack implementations
    void (*enqueue)(list *l, void *item); // same as add()
    void *(*dequeue)(list *l); // remove from front
    void (*push)(list *l, void *item); // same as add()
    void *(*pop)(list *l); // remove from end (last added)
    void *(*peek)(list *l);  // return last, without removing

    // iterator implementation is private. caller to free iterator
    iterator *(*create_iterator)(list *l);

    // involved operations
    void (*for_each)(list *l, visitor_func *visitor, void *extra_data);
    int (*find)(list *l, void *criteria, matcher_func *matcher, void *extra_data); // O(n)
    void (*append_all)(list *l, list *other);
    void (*sort)(list *l, comparer_func *comparer, void *extra_data);
    int (*bin_search)(list *l, void *criteria, comparer_func *comparer, void *extra_data);
    list *(*filter)(list *l, filter_func *filter, void *extra_data);
    list *(*map)(list *l, mapper_func *mapper, void *extra_data);
    void *(*reduce)(list *l, void *init_value, reducer_func *reduce, void *extra_data);
    list *(*deduplicate)(list *l, matcher_func *matcher, void *extra_data);
    string *(*join)(list *l, string *separator, to_string_func *to_str);
    unsigned long (*hash)(list *l, hasher_func *hasher);

    void (*free)(list *l, visitor_func *free_item); 
};

