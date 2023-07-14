#pragma once
#include <stdbool.h>
#include "../mempool.h"
#include "../unit_tests.h"
#include "../func_types.h"


typedef struct iterator {
    void *(*reset)(struct iterator *i);  // reset and return first item, if any
    bool  (*valid)(struct iterator *i);  // was the one last returned valid?
    void *(*next)(struct iterator *i);   // advance and return item, if any
    void *(*lookahead)(struct iterator *i, int times); // to enable an LL(n) parser
    void *private_data;
} iterator;


#define for_iterator(type, var, iter)           \
    for(type *var = (type *)iter->reset(iter);  \
        iter->valid(iter);                      \
        var = (type *)iter->next(iter))


// after getting an iterator, we can filter it, map it, sort it, group it,
// then: take one value of it, or reduce() to a value, or collect to a list

typedef struct hashtable hashtable;
typedef struct llist llist;

long iterator_count(iterator *it);
iterator *iterator_filter(iterator *it, filterer_func filter);
iterator *iterator_map(iterator *it, mapper_func filter);
hashtable *iterator_group(iterator *it, classifier_func classifier); // hashtable of llists per group
void *iterator_reduce(iterator *it, reducer_func filter, void *initial_value);
void *iterator_first(iterator *it);
void *iterator_last(iterator *it);
llist *iterator_collect(iterator *it, mempool *mp);

#ifdef INCLUDE_UNIT_TESTS
void iterator_unit_tests();
#endif
