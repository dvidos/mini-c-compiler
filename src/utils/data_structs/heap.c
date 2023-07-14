#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "heap.h"


heap *new_heap(mempool *mp, bool max, comparator_func *cmp);
void  heap_add(heap *h, void *item);  // O(lg N)
void  heap_delete(heap *h, void *item); // O(1)
void *heap_peek(heap *h);    // O(1)
void *heap_extract(heap *h); // O(1)

#ifdef INCLUDE_UNIT_TESTS
void heap_unit_tests() {
    // something something.
}
#endif

