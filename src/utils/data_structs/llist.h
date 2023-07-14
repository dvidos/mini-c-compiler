#pragma once
#include <stdbool.h>
#include "../mempool.h"
#include "../unit_tests.h"
#include "../func_types.h"
#include "iterator.h"


typedef struct llist llist;
typedef struct hashtable hashtable;


#define for_list(list, type, var)  \
        for(type *var = (type *)llist_iteration_reset(list);  \
            llist_iteration_valid(list);                      \
            var = (type *)llist_iteration_next(list))

llist *new_llist(mempool *mp);
llist *new_llist_of(mempool *mp, int num, ...);
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
int llist_find_first(llist *l, comparator_func *compare, void *item);
int llist_find_last(llist *l, comparator_func *compare, void *item);
llist *llist_sort(llist *l, comparator_func *compare, mempool *mp);
llist *llist_unique(llist *l, comparator_func *compare, mempool *mp);
llist *llist_filter(llist *l, filterer_func *filter, mempool *mp);
llist *llist_map(llist *l, mapper_func *map, mempool *mp);
void *llist_reduce(llist *l, reducer_func *reduce, void *initial_value, mempool *mp);
void *llist_iteration_reset(llist *l);
bool  llist_iteration_valid(llist *l);
void *llist_iteration_next(llist *l);
iterator *llist_create_iterator(llist *l, mempool *mp);
hashtable *llist_group(llist *l, classifier_func *classify, mempool *mp);

#ifdef INCLUDE_UNIT_TESTS
void llist_unit_tests();
#endif
