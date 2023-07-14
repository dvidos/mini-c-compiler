#pragma once
#include <stdbool.h>
#include "../mempool.h"
#include "../unit_tests.h"
#include "../func_types.h"


typedef struct heap heap; // for implementing priority queues

heap *new_heap(mempool *mp, bool max, comparator_func *cmp);
void  heap_add(heap *h, void *item);  // O(lg N)
void  heap_delete(heap *h, void *item); // O(1)
void *heap_peek(heap *h);    // O(1)
void *heap_extract(heap *h); // O(1)

#ifdef INCLUDE_UNIT_TESTS
void heap_unit_tests();
#endif
