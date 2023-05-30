#pragma once


// the following functions to perform operations in collections
// - item is the item under operation, 
// - extra_data is any runtime value to affect the operation


// can perform one operation on an item
typedef void visitor_func(void *item, void *extra_data);

// can compare two items and return <0, 0, >0, similar to strcmp()
typedef int comparer_func(const void *a, const void *b, void *extra_data);

// can provide a boolean value, whether an item passes a filter
typedef bool filter_func(void *item, void *extra_data);

// can check whether an item matches some criteria
typedef bool matcher_func(const void *item, const void *criteria, void *extra_data);

// based on an item, it can return something new
typedef void *mapper_func(void *item, void *extra_data);

// based on an item and a previous value, derives a new value
typedef void *reducer_func(void *item, void *prev_value, void *extra_data);

// allocates and returns a string with the description of an item. caller to free() the string
typedef char *to_string_func(void *item);

// returns a hash value based on the item
typedef unsigned long hasher_func(void *item);






typedef struct iterator {
    void *(*reset)(struct iterator *i); // reset and return first item, if any
    bool (*valid)(struct iterator *i);  // was the one last returned valid?
    void *(*next)(struct iterator *i);  // advance and return item, if any
    void *private_data;
} iterator;

typedef struct visitor {
    void (*run)(struct visitor *v, void *data);
    void *private_data;
} visitor;

typedef struct comparer {
    int (*run)(struct comparer *c, void *item_a, void *item_b);
    void *private_data;
} comparer;

typedef struct filterer {
    bool (*run)(struct filterer *f, void *data);
    void *private_data;
} filterer;


