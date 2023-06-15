#pragma once
#include <stdbool.h>
#include "data_types.h"
#include "unit_tests.h"

// ----------------------------------------

typedef struct str str;
typedef struct hashtable hashtable;
typedef struct llist llist;


// maybe these could be promoted to interfaces, the same way the iterator is an interface
// we can also crate a data source and a data sink.
// maybe we are asking from our code to lean onto this framework a little too much...

typedef int comparator_function(const void *a, const void *b);
typedef bool filterer_function(void *item);
typedef void *mapper_function(void *item, mempool *mp);
typedef void *reducer_function(void *item, void *prev_value, mempool *mp);
typedef str *classifier_function(void *item);


// -------------------------------------------

typedef struct iterator {
    void *(*reset)(struct iterator *i); // reset and return first item, if any
    bool (*valid)(struct iterator *i);  // was the one last returned valid?
    void *(*next)(struct iterator *i);  // advance and return item, if any
    void *private_data;
} iterator;

#define for_iterator(type, var, iter)  \
    for(type *var = (type *)iter->reset(iter);  \
        iter->valid(iter);                      \
        var = (type *)iter->next(iter))

// after getting an iterator, we can filter it, map it, sort it, group it,
// then: take one value of it, or reduce() to a value, or collect to a list

long iterator_count(iterator *it);
iterator *iterator_filter(iterator *it, filterer_function filter);
iterator *iterator_map(iterator *it, mapper_function filter);
hashtable *iterator_group(iterator *it, classifier_function classifier); // hashtable of llists per group
void *iterator_reduce(iterator *it, reducer_function filter, void *initial_value);
void *iterator_first(iterator *it);
void *iterator_last(iterator *it);
llist *iterator_collect(iterator *it, mempool *mp);

// -------------------------------------------

typedef struct queue queue;

queue *new_queue(mempool *mp);
int   queue_length(queue *q);
bool  queue_is_empty(queue *q);
void  queue_clear(queue *q);
void  queue_put(queue *q, void *item);
void *queue_peek(queue *q);
void *queue_get(queue *q);

// -------------------------------------------

typedef struct stack stack;

stack *new_stack(mempool *mp);
int   stack_length(stack *s);
bool  stack_is_empty(stack *s);
void  stack_clear(stack *s);
void  stack_push(stack *s, void *item);
void *stack_peek(stack *s);
void *stack_pop(stack *s);

// -------------------------------------------

typedef struct llist llist;

llist *new_llist(mempool *mp);
int   llist_length(llist *l);
bool  llist_is_empty(llist *l);
void *llist_get(llist *l, int index);
void  llist_add(llist *l, void *item);
void  llist_add_all(llist *l, llist *source);
bool  llist_contains(llist *l, void *item);
int   llist_index_of(llist *l, void *item);
bool  llist_insert_at(llist *l, int index, void *item);
bool  llist_remove_at(llist *l, int index);
void  llist_clear(llist *l);
llist *llist_reverse(llist *l);
int llist_find_first(llist *l, comparator_function *compare, void *item);
int llist_find_last(llist *l, comparator_function *compare, void *item);
llist *llist_sort(llist *l, comparator_function *compare, mempool *mp);
llist *llist_unique(llist *l, comparator_function *compare, mempool *mp);
llist *llist_filter(llist *l, filterer_function *filter, mempool *mp);
llist *llist_map(llist *l, mapper_function *map, mempool *mp);
void *llist_reduce(llist *l, reducer_function *reduce, void *initial_value, mempool *mp);
iterator *llist_create_iterator(llist *l, mempool *mp);
hashtable *llist_group(llist *l, classifier_function *classify, mempool *mp);

// -------------------------------------------

typedef struct bstree bstree;
typedef struct iterator iterator;

bstree *new_bstree(mempool *mp);
int   bstree_length(bstree *t);
bool  bstree_empty(bstree *t);
void *bstree_get(bstree *t, str *key);
bool  bstree_put(bstree *t, str *key, void *data);
bool  bstree_delete(bstree *t, str *key);
void  bstree_clear(bstree *t);
bool  bstree_contains(bstree *t, str *key);
iterator *bstree_create_iterator(bstree *t, mempool *m);
void bstree_print(bstree *t, FILE *f);

// -------------------------------------------

typedef struct hashtable hashtable;

hashtable *new_hashtable(mempool *mp, int capacity);
int   hashtable_length(hashtable *h);
bool  hashtable_is_empty(hashtable *h);
void  hashtable_set(hashtable *h, str *key, void *data); // O(1)
void *hashtable_get(hashtable *h, str *key); // O(1)
bool  hashtable_delete(hashtable *h, str *key); // O(1)
bool  hashtable_contains(hashtable *h, str *key); // O(1)
void  hashtable_clear(hashtable *h);
iterator *hashtable_create_iterator(hashtable *h, mempool *m);

// -------------------------------------------

typedef struct heap heap; // for implementing priority queues

heap *new_heap(mempool *mp, bool max, comparator_function *cmp);
void  heap_add(heap *h, void *item);  // O(lg N)
void  heap_delete(heap *h, void *item); // O(1)
void *heap_peek(heap *h);    // O(1)
void *heap_extract(heap *h); // O(1)

// -------------------------------------------

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

// -------------------------------------------

#ifdef INCLUDE_UNIT_TESTS
    void all_data_structs_unit_tests();
#endif
