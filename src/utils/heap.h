#pragma once
#include "string.h"
#include "iterator.h"

struct heap_vtable;

// a binary search tree, operations usually in O(lg N) time
typedef struct heap {
    void *priv_data;
    struct heap_vtable *v;
} heap;

heap *new_heap(bool is_max_heap);

struct heap_vtable {
    int (*count)(heap *h);
    void (*clear)(heap *h);

    void (*put)(heap *h, string *key, void *value); // O(lg N) operation
    void *(*peek_minmax)(heap *h);    // O(1) operation
    void *(*extract_minmax)(heap *h); // O(1) operation

    void (*free)(heap *h, visitor_func *freeer);
};

// -----------------------------

struct heap_node {
    string *key;
    void *value;
    struct heap_node *left;
    struct heap_node *right;
    struct heap_node *parent;
};

struct heap_priv_data {
    struct heap_node *root;
    bool is_max_heap;
    int count;
};
