#pragma once
#include <stdbool.h>
#include "../mempool.h"
#include "../unit_tests.h"
#include "../func_types.h"
#include "iterator.h"


typedef struct list list;
typedef struct hashtable hashtable;


#define for_list(list, type, var)  \
        for(type *var = (type *)list_iteration_reset(list);  \
            list_iteration_valid(list);                      \
            var = (type *)list_iteration_next(list))

list *new_list(mempool *mp);
list *new_list_of(mempool *mp, int num, ...);
int   list_length(list *l);
bool  list_is_empty(list *l);
void *list_get(list *l, int index);
void  list_add(list *l, void *item);
void  list_add_all(list *l, list *source);
bool  list_contains(list *l, void *item);
int   list_index_of(list *l, void *item);
bool  list_insert_at(list *l, int index, void *item);
bool  list_remove_at(list *l, int index);
void  list_clear(list *l);
list *list_reverse(list *l);
int list_find_first(list *l, comparator_func *compare, void *item);
int list_find_last(list *l, comparator_func *compare, void *item);
list *list_sort(list *l, comparator_func *compare, mempool *mp);
list *list_unique(list *l, comparator_func *compare, mempool *mp);
list *list_filter(list *l, filterer_func *filter, mempool *mp);
list *list_map(list *l, mapper_func *map, mempool *mp);
void *list_reduce(list *l, reducer_func *reduce, void *initial_value, mempool *mp);
void *list_iteration_reset(list *l);
bool  list_iteration_valid(list *l);
void *list_iteration_next(list *l);
iterator *list_create_iterator(list *l, mempool *mp);
hashtable *list_group(list *l, classifier_func *classify, mempool *mp);

#ifdef INCLUDE_UNIT_TESTS
void list_unit_tests();
#endif
