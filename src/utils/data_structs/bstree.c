#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "bstree.h"
#include "stack.h"


typedef struct bstree_node {
    str *key;
    void *data;
    struct bstree_node *left;
    struct bstree_node *right;
    struct bstree_node *parent;
} bstree_node;

struct bstree {
    int nodes_count;
    bstree_node *root;
    mempool *mempool; // for allocating new nodes
};

bstree *new_bstree(mempool *mp) {
    bstree *t = mpalloc(mp, bstree);
    t->mempool = mp;
    return t;
}

int bstree_length(bstree *t) {
    return t->nodes_count;
}

bool bstree_empty(bstree *t) {
    return t->nodes_count == 0;
}

bool bstree_contains(bstree *t, str *key) {
    bstree_node *n = t->root;
    while (n != NULL) {
        int c = str_cmp(key, n->key);
        if      (c == 0) return true;
        else if (c  < 0) n = n->left;
        else if (c  > 0) n = n->right;
    }
    return false;
}

void *bstree_get(bstree *t, str *key) { // O(lg N) time complexity (1 million nodes -> 20 hops)
    bstree_node *n = t->root;
    while (n != NULL) {
        int c = str_cmp(key, n->key);
        if      (c == 0) return n->data;
        else if (c  < 0) n = n->left;
        else if (c  > 0) n = n->right;
    }
    return n;
}

bool bstree_put(bstree *t, str *key, void *data) {
    bstree_node *new_node = mpalloc(t->mempool, bstree_node);
    memset(new_node, 0, sizeof(bstree_node));
    new_node->key = key;
    new_node->data = data;

    if (t->root == NULL) {
        t->root = new_node;
        t->nodes_count++;
        return true;
    }

    bstree_node *n = t->root;
    while (n != NULL) {
        int c = str_cmp(key, n->key);
        if (c == 0)
            return false; // key already in tree
        
        if (c < 0) {
            if (n->left == NULL) {
                n->left = new_node;
                t->nodes_count++;
                return true;
            }
            n = n->left;
        } else if (c > 0) {
            if (n->right == NULL) {
                n->right = new_node;
                t->nodes_count++;
                return true;
            }
            n = n->right;
        }
    }

    // we should not get here
    return false;
}

static void bstree_remove_node(bstree_node *node, bstree_node **pointer_to_node) {
    if (node->left == NULL && node->right == NULL) {
        // node is a leaf, just remove it.
        *pointer_to_node = NULL;

    } else if (node->left != NULL && node->right == NULL) {
        // node only has left node, bring it up to parent
        *pointer_to_node = node->left;
        node->left = NULL;

    } else if (node->left == NULL && node->right != NULL) {
        // node only has right node, bring it up to parent
        *pointer_to_node = node->right;
        node->right = NULL;

    } else {
        // node has two child nodes.
        // from the right subtree, find the smallest key node. this is the successor
        // put its key & data on the node under deletion
        // and remove this min node from its parent (it may have at most one node)
        bstree_node **pointer_to_successor = &node->right;
        bstree_node *successor = node->right;
        while (successor->left != NULL) {
            pointer_to_successor = &successor->left;
            successor = successor->left;
        }
        // copy key & data to the node to be deleted
        node->key = successor->key;
        node->data = successor->data;
        // remove that successor
        bstree_remove_node(successor, pointer_to_successor);
    }
}

bool bstree_delete(bstree *t, str *key) {

    // keep a trailing pointer to the pointer that points the node to delete
    bstree_node *node = t->root;
    bstree_node **pointer_to_node = &t->root;
    while (node != NULL) {
        int c = str_cmp(key, node->key);
        if (c == 0) {
            bstree_remove_node(node, pointer_to_node);
            t->nodes_count--;
            return true;
        }
        else if (c < 0) {
            if (node->left == NULL)
                return false; // not found

            pointer_to_node = &node->left;
            node = node->left;
        }
        else if (c > 0) {
            if (node->right == NULL)
                return false; // not found

            pointer_to_node = &node->right;
            node = node->right;
        }
    }

    return false;
}

void bstree_clear(bstree *t) {
    t->root = NULL;
    t->nodes_count = 0;
}

typedef struct bstree_iterator_private_state {
    bstree *tree;
    stack *stack;  // stack holds bstree_nodes. the stack top is the iterator's current.
    bool last_operation_valid;
} bstree_iterator_private_state;

static void *bstree_iterator_reset(iterator *it) {
    bstree_iterator_private_state *it_state = (bstree_iterator_private_state *)it->private_data;

    stack_clear(it_state->stack);
    if (it_state->tree->root == NULL)
        return NULL;
    
    // find the smallest node
    bstree_node *n = it_state->tree->root;
    while (n != NULL) {
        stack_push(it_state->stack, n);
        n = n->left;
    }

    return ((bstree_node *)stack_peek(it_state->stack))->data;
}

static bool bstree_iterator_valid(iterator *it) {
    bstree_iterator_private_state *it_state = (bstree_iterator_private_state *)it->private_data;
    return stack_peek(it_state->stack) != NULL;
}

static void *bstree_iterator_next(iterator *it) {
    bstree_iterator_private_state *it_state = (bstree_iterator_private_state *)it->private_data;

    // if there was no node so far, there's nowhere to go from here
    bstree_node *curr = stack_peek(it_state->stack);
    if (curr == NULL)
        return NULL;
    
    // if there is a right child, go to the smallest key of the right subtree
    if (curr->right != NULL) {
        curr = curr->right;
        stack_push(it_state->stack, curr);
        while (curr->left != NULL) {
            curr = curr->left;
            stack_push(it_state->stack, curr);
        }
        return curr->data;
    }

    // as there is no right child, we need to move up the tree to find something larger.
    curr = stack_pop(it_state->stack);

    // if stack is empty, it means we popped the root, and there was no right child.
    if (stack_is_empty(it_state->stack)) {
        return NULL;
    }

    // if we are the left child of the parent, they are the next in order.
    bstree_node *parent = stack_peek(it_state->stack);
    if (curr == parent->left) {
        return parent->data;
    }

    // so we were the right child of the parent. we are done, they are done as well.  
    // we must go up, all the way until we find a parent whose we are the left child.
    // if we find no such parent, it means we were the rightmost node, hence the last one
    curr = stack_pop(it_state->stack);
    while (!stack_is_empty(it_state->stack)) {
        parent = stack_peek(it_state->stack);
        if (curr == parent->left) {
            // we were the left child of this parent, they are next in order.
            return parent->data;
        }
        // so we were the right child of this parent, we must continue up
        curr = stack_pop(it_state->stack);
    }
    
    // we emptied the stack. this means we don't have a parent to go up to.
    // we must have been the rightmost child of all. this is the end.
    return NULL;
}

static void *bstree_iterator_lookahead(iterator *it, int times) {
    // not supported for now
    return NULL;
}

iterator *bstree_create_iterator(bstree *t, mempool *mp) {
    bstree_iterator_private_state *it_state = mpalloc(mp, bstree_iterator_private_state);
    it_state->tree = t;
    it_state->stack = new_stack(mp);

    iterator *it = mpalloc(mp, iterator);
    it->private_data = it_state;
    it->reset = bstree_iterator_reset;
    it->valid = bstree_iterator_valid;
    it->next = bstree_iterator_next;
    it->lookahead = bstree_iterator_lookahead;
    return it;
}

static void bstree_print_node(bstree_node *n, int depth, char prefix, FILE *f) {
    if (n == NULL)
        return;
    fprintf(f, "%*s%c: %s\n", depth * 4, "", prefix, str_charptr(n->key));
    bstree_print_node(n->left, depth + 1, 'L', f);
    bstree_print_node(n->right, depth + 1, 'R', f);
}

void bstree_print(bstree *t, FILE *f) {
    bstree_print_node(t->root, 0, 'R', f);
}

static void bstree_compact_string_node(bstree_node *n, str *s) {
    if (n == NULL)
        return;

    if (n->left == NULL && n->right == NULL) {
        str_cat(s, n->key);
    } else {
        str_cats(s, "(");
        str_cat(s, n->key);
        str_cats(s, ",");
        bstree_compact_string_node(n->left, s);
        str_cats(s, ",");
        bstree_compact_string_node(n->right, s);
        str_cats(s, ")");
    }
}

str *bstree_compact_string(bstree *t, mempool *mp) {
    str *s = new_str(mp, "");
    bstree_compact_string_node(t->root, s);
    return s;
}

#ifdef INCLUDE_UNIT_TESTS
static bstree *bstree_create_unit_test_tree(mempool *mp) {
    /*
        Create the following tree, to be used in testing.
        
        |      E
        |     / \
        |    B   F
        |   / \   \
        |  A   D   G
        |     /
        |    C
    */
    bstree *t = new_bstree(mp);
    bstree_put(t, new_str(mp, "E"), "E");
    bstree_put(t, new_str(mp, "B"), "B");
    bstree_put(t, new_str(mp, "F"), "F");
    bstree_put(t, new_str(mp, "A"), "A");
    bstree_put(t, new_str(mp, "D"), "D");
    bstree_put(t, new_str(mp, "C"), "C");
    bstree_put(t, new_str(mp, "G"), "G");

    return t;
}

void bstree_unit_tests() {
    mempool *mp = new_mempool();

    str *key_a = new_str(mp, "key a");
    str *key_b = new_str(mp, "key b");
    str *key_c = new_str(mp, "key c");
    char *data_a = "data a";
    char *data_b = "data b";
    char *data_c = "data c";
    str *key_z = new_str(mp, "key z");
    void *p;

    bstree *t = new_bstree(mp);
    assert(bstree_length(t) == 0);
    assert(bstree_empty(t));

    // add root
    bstree_put(t, key_b, data_b);
    assert(bstree_length(t) == 1);
    assert(!bstree_empty(t));
    assert(strcmp(str_charptr(t->root->key), "key b") == 0);

    // simple addition, make sure left/right
    bstree_put(t, key_a, data_a);
    bstree_put(t, key_c, data_c);
    assert(bstree_length(t) == 3);
    assert(strcmp(str_charptr(t->root->left->key), "key a") == 0);
    assert(strcmp(str_charptr(t->root->right->key), "key c") == 0);
    assert(strcmp(t->root->left->data, "data a") == 0);
    assert(strcmp(t->root->right->data, "data c") == 0);
    assert(t->root->left->left == NULL);
    assert(t->root->left->right == NULL);
    assert(t->root->left->left == NULL);
    assert(t->root->right->right == NULL);
    
    // verify find / contains
    assert(bstree_contains(t, key_c));
    p = bstree_get(t, key_c);
    assert(p != NULL);
    assert(strcmp(p, "data c") == 0);
    assert(!bstree_contains(t, key_z));
    p = bstree_get(t, key_z);
    assert(p == NULL);


    // test deletion, case 1: delete a leaf node
    t = bstree_create_unit_test_tree(mp);
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,(D,C,)),(F,,G))") == 0);
    bstree_delete(t, new_str(mp, "C"));
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,D),(F,,G))") == 0);

    // test deletion, case 2: delete a node that has only left child
    t = bstree_create_unit_test_tree(mp);
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,(D,C,)),(F,,G))") == 0);
    bstree_delete(t, new_str(mp, "D"));
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,C),(F,,G))") == 0);

    // test deletion, case 3: delete a node that has only right child
    t = bstree_create_unit_test_tree(mp);
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,(D,C,)),(F,,G))") == 0);
    bstree_delete(t, new_str(mp, "F"));
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,(D,C,)),G)") == 0);

    // test deletion, case 4: delete a node that has two children
    t = bstree_create_unit_test_tree(mp);
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,(D,C,)),(F,,G))") == 0);
    bstree_delete(t, new_str(mp, "B"));
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(C,A,D),(F,,G))") == 0);

    // test deletion, case 4+: delete root node
    t = bstree_create_unit_test_tree(mp);
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,(D,C,)),(F,,G))") == 0);
    bstree_delete(t, new_str(mp, "E"));
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(F,(B,A,(D,C,)),G)") == 0);


    // test iterator, traverse the tree, collect payloads
    t = bstree_create_unit_test_tree(mp);
    str *series = new_str(mp, "");
    iterator *it = bstree_create_iterator(t, mp);
    for_iterator(char, p, it)
        str_cats(series, p);
    assert(strcmp(str_charptr(series), "ABCDEFG") == 0);


    // in mem pool: 240 allocations, 12K bytes total, and not a single byte leaking!
    mempool_release(mp);
}
#endif

