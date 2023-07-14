#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "iterator.h"
#include "llist.h"


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

iterator *iterator_filter(iterator *it, filterer_func filter);
iterator *iterator_map(iterator *it, mapper_func filter);
void *iterator_reduce(iterator *it, reducer_func filter, void *initial_value);

hashtable *iterator_group(iterator *it, classifier_func classifier); // hashtable of llists per group

void *iterator_first(iterator *it) {
    void *item = it->reset(it);
    if (it->valid(it))
        return item;
    return NULL;
}

void *iterator_last(iterator *it) {
    void *valid_item = NULL;
    void *item = it->reset(it);
    while (it->valid(it)) {
        valid_item = item;
        item = it->next(it);
    }
    return valid_item;
}

llist *iterator_collect(iterator *it, mempool *mp) {
    llist *l = new_llist(mp);
    for_iterator(void, ptr, it)
        llist_add(l, ptr);
    return l;
}

#ifdef INCLUDE_UNIT_TESTS
void iterator_unit_tests() {
    // test the following
    // long iterator_count(iterator *it);
    // iterator *iterator_filter(iterator *it, filterer_func filter);
    // iterator *iterator_map(iterator *it, mapper_func filter);
    // hashtable *iterator_group(iterator *it, classifier_func classifier); // hashtable of llists per group
    // void *iterator_reduce(iterator *it, reducer_func filter, void *initial_value);
    // void *iterator_first(iterator *it);
    // void *iterator_last(iterator *it);
    // llist *iterator_collect(iterator *it, mempool *mp);
}
#endif

