#pragma once
#include "func_types.h"



struct hashtable_vtable;

typedef struct hashtable {
    void *priv_data;
    struct hashtable_vtable *v;
} hashtable;

hashtable *new_hashtable(int capacity);

struct hashtable_vtable {
    int (*count)(hashtable *h);
    void (*clear)(hashtable *h);

    bool (*contains)(hashtable *h, string *key);     // O(1) operation
    void (*set)(hashtable *h, string *key, void *value); // O(1) operation
    void *(*get)(hashtable *h, string *key);             // O(1) operation
    void (*delete)(hashtable *h, string *key);           // O(1) operation

    // we can create a string/value iterator, strongly typed, akin to array

    // caller to free arrays
    array *(*get_keys)(hashtable *h);
    array *(*get_values)(hashtable *h);

    void (*free)(hashtable *h);
};

