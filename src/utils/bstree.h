#pragma once
#include "string.h"
#include "iterator.h"

struct bstree_vtable;

// a binary search tree, operations usually in O(lg N) time
typedef struct bstree {
    void *priv_data;
    struct bstree_vtable *v;
} bstree;

bstree *new_bstree();

struct bstree_vtable {
    int (*count)(bstree *t);
    void (*clear)(bstree *t);

    bool (*contains)(bstree *t, string *key);         // O(lg N) operation
    void (*put)(bstree *t, string *key, void *value); // O(lg N) operation (overwrites or adds)
    void *(*get)(bstree *t, string *key);             // O(lg N) operation
    void (*delete)(bstree *t, string *key);

    void *(*get_min)(bstree *t); // O(lg N) operation (min key's value)
    void *(*get_max)(bstree *t); // O(lg N) operation
    void (*for_each)(bstree *t, visitor_func *visitor, void *extra_data);

    // iterator implementation is private. caller to free iterator
    iterator *create_iterator(bstree *t);

    void (*free)(bstree *t, visitor_func *freeer);
};

// -----------------------------

struct bstree_node {
    string *key;
    void *value;
    struct bstree_node *left;
    struct bstree_node *right;
};

struct bstree_priv_data {
    struct bstree_node *root;
    int count;
};
