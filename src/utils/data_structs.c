#include <string.h>
#include "../unit_tests.h"
#include "data_structs.h"
#include "mempool.h"

// ------------------------------------------------

typedef struct queue_node {
    void *item;
    struct queue_node *next;
} queue_node;

struct queue {
    int items;
    queue_node *head;
    queue_node *tail;
    mempool *mempool;
};

queue *new_queue(mempool *mp) {
    queue *q = mempool_alloc(mp, sizeof(queue), "queue");
    memset(q, 0, sizeof(queue));
    q->mempool = mp;
    return q;
}

int queue_length(queue *q) {
    return l->items;
}

bool queue_empty(queue *q) {
    return l->items == 0;
}

void queue_put(queue *q, void *item) {
    queue_node *n = mempool_alloc(q->mempool, sizeof(queue_node), "queue_node");
    n->item = item;
    n->next = NULL;

    if (q->head == NULL) {
        q->head = n;
        q->tail = n;
        q->items = 1;
    } else {
        q->tail->next = n;
        q->tail = n;
        q->items++;
    }
}

void *queue_peek(queue *q) {
    return q->head == NULL ? NULL : q->head->item;
}

void *queue_get(queue *q) {
    if (q->head == NULL)
        return NULL;

    queue_node *n = q->head;  // what a joy to not have to free() !!!
    if (n == q->tail) {
        // was the only item
        q->head = NULL;
        q->tail = NULL;
        q->items = 0;
    } else {
        q->head = q->head->next;
        q->items--;
    }

    return n->item; 
}

void queue_unit_tests() {
    // test the following:
    // queue *new_queue(mempool *mp);
    // void  queue_length(queue *q);
    // bool  queue_empty(queue *q);
    // void queue_put(queue *q, void *item);
    // void *queue_peek(queue *q);
    // void *queue_get(queue *q);
}

// -------------------------------------------

typedef struct stack_node {
    void *item;
    struct stack_node *next;
} stack_node;

struct stack {
    int items;
    stack_node *top; // where we push and pop
    mempool *mempool;
};

stack *new_stack(mempool *mp) {
    stack *s = mempool_alloc(mp, sizeof(stack), "stack");
    memset(s, 0, sizeof(stack));
    s->mempool = mp;
    return s;
}

int stack_length(stack *s) {
    return l->items;
}

bool stack_empty(stack *s) {
    return s->items == 0;
}

void stack_push(stack *s, void *item) {
    stack_node *n = mempool_alloc(s->mempool, sizeof(stack_node), "stack_node");
    n->item = item;
    n->next = NULL;

    if (s->top == NULL) {
        s->top = n;
        s->items = 1;
    } else {
        n->next = s->top;
        s->top = n;
        s->items++;
    }
}

void *stack_peek(stack *s) {
    return s->top == NULL ? NULL : s->top->item;
}

void *stack_pop(stack *s) {
    if (s->top == NULL)
        return NULL;

    stack_node *n = s->top;  // what a joy to not have to free() !!!
    if (n == s->top) {
        // was the only item
        s->top = NULL;
        s->items = 0;
    } else {
        s->top = s->top->next;
        s->items--;
    }

    return n->item; 
}

void stack_unit_tests() {
    // test the following:
    // stack *new_stack(mempool *mp);
    // void  stack_length(stack *s);
    // bool  stack_empty(stack *s);
    // void stack_put(stack *s, void *item);
    // void *stack_peek(stack *s);
    // void *stack_get(stack *s);
}

// -------------------------------------------

typedef struct llist_node {
    void *data;
    struct llist_node *next;
    struct llist_node *prev;
};

struct llist {
    int items_count;
    struct llist_node *head;
    struct llist_node *tail;
};

llist *new_llist(mempool *mp);
void  llist_length(llist *l);
bool  llist_empty(llist *l);
void *llist_get(llist *l, int index);
void  llist_add(llist *l, void *item);
void  llist_push(llist *l, void *item);
void *llist_peek(llist *l);
void *llist_pop(llist *l);
void  llist_insert_head(llist *l, void *item);
void *llist_extract_head(llist *l);
bool  llist_contains(llist *l, void *item);
int   llist_index_of(llist *l, void *item);
bool  llist_remove_at(llist *l, int index);
llist *llist_reverse(llist *l);
llist *llist_sort(llist *l, comparator_function *c);
llist *llist_unique(llist *l, comparator_function *c);
llist *llist_filter(llist *l, filterer_function *f);
llist *llist_map(llist *l, mapper_function *m);
void *llist_reduce(llist *l, reducer_function *r, void *initial_value);
iterator *llist_create_iterator(llist *l, mempool *m);
hashtable *llist_group(llist *l, classifier_function *c)

void llist_unit_tests() {
    // test the following:
    // llist *new_llist(mempool *mp);
    // void  llist_length(llist *l);
    // bool  llist_empty(llist *l);
    // void *llist_get(llist *l, int index);
    // void  llist_add(llist *l, void *item);
    // void  llist_push(llist *l, void *item);
    // void *llist_peek(llist *l);
    // void *llist_pop(llist *l);
    // void  llist_insert_head(llist *l, void *item);
    // void *llist_extract_head(llist *l);
    // bool  llist_contains(llist *l, void *item);
    // int   llist_index_of(llist *l, void *item);
    // bool  llist_remove_at(llist *l, int index);
    // llist *llist_reverse(llist *l);
    // llist *llist_sort(llist *l, comparator_function *c);
    // llist *llist_unique(llist *l, comparator_function *c);
    // llist *llist_filter(llist *l, filterer_function *f);
    // llist *llist_map(llist *l, mapper_function *m);
    // void *llist_reduce(llist *l, reducer_function *r, void *initial_value);
    // iterator *llist_create_iterator(llist *l, mempool *m);
    // hashtable *llist_group(llist *l, classifier_function *c)
}

// ------------------------------------------------------------------

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
    bstree *t = mempool_alloc(mp, sizeof(bstree), "bstree");
    t->mempool = mp;
    memset(t, 0, sizeof(bstree));
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

void *bstree_get(bstree *t, str *key) { // O(lg N) time complexity (1 million entries -> 20 hops)
    bstree_node *n = t->root;
    while (n != NULL) {
        int c = str_cmp(key, n->key);
        if      (c == 0) return n->data;
        else if (c  < 0) n = n->left;
        else if (c  > 0) n = n->right;
    }
    return n;
}

bool bstree_insert(bstree *t, str *key, void *data) {
    bstree_node *node = mempool_alloc(t->mempool, sizeof(bstree_node), "bstree_node");
    node->key = key;
    node->data = data;

    ...

    // find new parent
    // insert left or right.
    t->nodes_count++;
    return false;
}

bool bstree_delete(bstree *t, str *key) {
    // find target
    // adjust children accordingly

    ...

    t->nodes_count--;
    return true;
}

typedef struct bstree_iterator_private_data {
    bstree *tree;
    bstree_node *curr;
    bool last_operation_valid;
} bstree_iterator_private_data;

static void *bstree_iterator_reset(iterator *it) {
    bstree_iterator_private_data *data = (bstree_iterator_private_data *)it->private_data;

    // find the smallest node
    data->curr = data->tree->root;
    while (data->curr != NULL && data->curr->left != NULL)
        data->curr = data->curr->left;
    data->last_operation_valid = (data->curr != NULL);

    return data->curr;
}

static bool bstree_iterator_valid(iterator *it) {
    bstree_iterator_private_data *data = (bstree_iterator_private_data * *)it->private_data;
    return data->last_operation_valid;
}

static void *bstree_iterator_next(iterator *it) {
    bstree_iterator_private_data *data = (bstree_iterator_private_data * *)it->private_data;

    // if there is one on the right, go right,
    // otherwise, go up, to the first node whose we are the left child.
    // if there is none, we finished traversing the tree

    ...
}

iterator *bstree_create_iterator(bstree *t, mempool *m) {
    bstree_iterator_private_data *data = mempool_alloc(mp, sizeof(bstree_iterator_private_data), "bstree_iterator_private_data");
    memset(data, 0, sizeof(data));
    data->tree = t;

    iterator *it = mempool_alloc(mp, sizeof(iterator), "iterator");
    it->private_data = data;
    it->reset = bstree_iterator_reset;
    it->valid = bstree_iterator_valid;
    it->next = bstree_iterator_next;
    return it;
}

void bstree_unit_tests() {
    // test the following:
    // bstree *new_bstree(mempool *mp);
    // int   bstree_length(bstree *t);
    // bool  bstree_empty(bstree *t);
    // void *bstree_get(bstree *t, str *key);
    // bool  bstree_insert(bstree *t, str *key, void *data);
    // bool  bstree_delete(bstree *t, str *key);
    // bool  bstree_contains(bstree *t, str *key);
    // iterator *bstree_create_iterator(bstree *t, mempool *m);
}

// --------------------------------------------------------

typedef struct hashtable_node {
    str *key;
    void *data;
    struct hashtable_node *next;
} hashtable_node;

struct hashtable {
    int capacity;
    int items_count;
    hashtable_node **items_arr;  // the array of node pointers
    mempool *mempool; // for creating nodes
};

hashtable *new_hashtable(mempool *mp, int capacity) {
    hashtable *h = mempool_alloc(mp, sizeof(hashtable), "hashtable");
    memset(h, 0, sizeof(hashtable));
    h->capacity = capacity;
    h->items_arr = (hashtable_node **)mempool_alloc(mp, capacity * sizeof(void *), "hashtable_items_arr");
    h->mempool = mp;
    return h;
}

int hashtable_length(hashtable *h) {
    return h->items_count;
}

bool hashtable_empty(hashtable *h) {
    return h->items_count == 0;
}

bool hashtable_contains(hashtable *h, str *key) { // O(1)
    int slot = (int)(str_hash(key) % h->capacity);
    hashtable_node *n = h->items_arr[slot];
    while (n != NULL) {
        if (str_cmp(n->key, key) == 0)
            return true;
        n = n->next;
    }
    return false;
}

void *hashtable_get(hashtable *h, str *key) { // O(1)
    int slot = (int)(str_hash(key) % h->capacity);
    hashtable_node *n = h->items_arr[slot];
    while (n != NULL) {
        if (str_cmp(n->key, key) == 0)
            return n->data;
        n = n->next;
    }
    return NULL;
}

static hashtable_node *hashtable_create_node(hashtable *h, str *key, void *data) {
    hashtable_node *node = mempool_alloc(h->mempool, sizeof(hashtable_node), "hashtable_node");
    node->key = key;
    node->data = data;
    node->next = NULL;
    return node;
}

void hashtable_set(hashtable *h, str *key, void *data) { // O(1)
    int slot = (int)(str_hash(key) % h->capacity);

    // easy solution if slot is empty
    if (h->items_arr[slot] == NULL) {
        h->items_arr[slot] = hashtable_create_node(h, key, data);
        h->items_count++;
        return;
    }

    // go over list, check if it exists, replace or append
    hashtable_node *curr = h->items_arr[slot];
    while (curr != NULL) {
        if (str_cmp(curr->key, key) == 0) {
            // we change data in place, without changing the items_count
            curr->data = data;
            return;
        }
        if (curr->next == NULL) {
            // this is the last item
            curr->next = hashtable_create_node(h, key, data);
            h->items_count++;
            return;
        }
        curr = curr->next;
    }
}

bool hashtable_delete(hashtable *h, str *key) { // O(1)
    int slot = (int)(str_hash(key) % h->capacity);

    hashtable_node *n = h->items_arr[slot];
    if (n == NULL)
        return false;

    ...
}

iterator *hashtable_create_iterator(hashtable *h, mempool *m) {

}

void hashtable_unit_tests() {
    // test the following:
    // hashtable *new_hashtable(mempool *mp);
    // int   hashtable_length(hashtable *h);
    // bool  hashtable_empty(hashtable *h);
    // void *hashtable_get(hashtable *h, str *key); // O(1)
    // bool  hashtable_insert(hashtable *h, str *key, void *data); // O(1)
    // insert simple, insert with mulitple per slot, insert + override

    // bool  hashtable_delete(hashtable *h, str *key); // O(1)
    // delete from slot, delete from nodes chain of same slot.

    // bool  hashtable_contains(hashtable *h, str *key); // O(1)
    // iterator *hashtable_create_iterator(hashtable *h, mempool *m);

}

// --------------------------------------------------------

typedef struct heap heap; // for implementing priority queues
heap *new_heap(mempool *mp, bool max, comparator_function *cmp);
void  heap_add(heap *h, void *item);  // O(lg N)
void  heap_delete(heap *h, void *item); // O(1)
void *heap_peek(heap *h);    // O(1)
void *heap_extract(heap *h); // O(1)
void heap_unit_tests();




typedef struct graph graph;
typedef struct graph_node graph_node;
graph *new_graph(mempool *mp);
int    graph_length(graph *g);
bool   graph_empty(graph *g);
bool   graph_add(graph *g, str *key, void *node_data);
bool   graph_add_link(graph *g, str *key_from, str *key_to, void *vertex_data);
void  *graph_get(graph *g, str *key);
llist *graph_get_neighbors(graph *g, str *node_key);
bool   graph_acyclic(graph *g);
int    graph_number_of_trees(graph *g);
llist *graph_topological_sort(graph *g);
llist *graph_shortest_path(graph *g, str *from_key, str *to_key);
iterator *graph_create_bfs_iterator(graph *g, mempool *m, str *first_key);
iterator *graph_create_dfs_iterator(graph *g, mempool *m, str *first_key);
void   graph_unit_tests();





// we should be able to filter, map, reduce, group, collect in a list, etc using only an iterator!
// maybe we can chain iterators one on another and create conversions using window functions
// llist, bstree, hashtable, generator, etc
typedef struct iterator {
    void *(*reset)(struct iterator *i); // reset and return first item, if any
    bool (*valid)(struct iterator *i);  // was the one last returned valid?
    void *(*next)(struct iterator *i);  // advance and return item, if any
    void *private_data;
} iterator;

#define iterate(type, var, iter)  \
    for(type *var = (type *)iter->reset(iter);  \
        iter->valid(iter);                      \
        var = (type *)iter->next(iter))

// load everything in a list, create iterator of the list,
// filter it,
// map it,
// sort it,
// group it,
// then: take one value, or reduce() to a value, or collect to a list

long iterator_count(iterator *it) {
    long count = 0;
    for (it->reset(it); it->valid(it); it->next(it))
        count++;
    return count;
}

iterator *iterator_filter(iterator *it, filterer_function filter);
iterator *iterator_map(iterator *it, mapper_function filter);
hashtable *iterator_group(iterator *it, classifier_function classifier); // hashtable of llists per group
void *iterator_reduce(iterator *it, reducer_function filter, void *initial_value);

void *iterator_first(iterator *it) {
    void *item = it->reset(it);
    if (it->valid(it))
        return item;
    return NULL;
}

void *iterator_last(iterator *it) {
    void *item = NULL;
    void *prev_item = NULL;

    void *item = it->reset(it);
    while (it->valid(it)) {
        prev_item = item;
        item = it->next(it);
    }

    return prev_item;
}

llist *iterator_collect(iterator *it, mempool *mp);
void iterator_unit_tests() {
    // test the following
    // long iterator_count(iterator *it);
    // iterator *iterator_filter(iterator *it, filterer_function filter);
    // iterator *iterator_map(iterator *it, mapper_function filter);
    // hashtable *iterator_group(iterator *it, classifier_function classifier); // hashtable of llists per group
    // void *iterator_reduce(iterator *it, reducer_function filter, void *initial_value);
    // void *iterator_first(iterator *it);
    // void *iterator_last(iterator *it);
    // llist *iterator_collect(iterator *it, mempool *mp);
}

// ---------------------------------------------------

void all_data_structs_unit_tests() {
    queue_unit_tests();
    stack_unit_tests();
    llist_unit_tests();
    bstree_unit_tests();
    hashtable_unit_tests();
    heap_unit_tests();
    graph_unit_tests();
    iterator_unit_tests();
}
