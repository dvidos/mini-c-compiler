#pragma once
#include "func_types.h"
#include "string.h"
#include "iterator.h"



struct hashtable_vtable;

typedef struct hashtable {
    void *priv_data;
    struct hashtable_vtable *v;
} hashtable;

hashtable *new_hashtable(int capacity);

struct hashtable_vtable {
    int (*count)(hashtable *h);
    void (*clear)(hashtable *h);

    bool (*contains)(hashtable *h, string *key);         // O(1) operation
    void (*set)(hashtable *h, string *key, void *value); // O(1) operation
    void *(*get)(hashtable *h, string *key);             // O(1) operation
    void (*delete)(hashtable *h, string *key);           // O(1) operation

    // caller to free arrays (how to duplicate values though?)
    array *(*get_keys)(hashtable *h);
    array *(*get_values)(hashtable *h);

    // iterator implementation is private. caller to free iterator
    iterator *create_keys_iterator(hashtable *h);

    void (*free)(hashtable *h, visitor_func *freeer);
};

// ----------------------

struct hashtable_entry {
    string *key;
    void *value;
    struct hashtable_entry *next;
};

struct hashtable_priv_data {
    struct hashtable_entry **entries_array;
    int entries_capacity;  // 
    int count;     // current items
};

