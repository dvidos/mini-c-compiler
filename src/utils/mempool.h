#pragma once
#include "../unit_tests.h"
#include <stddef.h>


// a memory pool (mempool for short) allows for the definition of objects "lifetime"
// any objects can be allocated from it, and all objects are free'd together at the end
// it grows indefinitely as demand grows
typedef struct mempool mempool;

mempool *new_mempool();
void *mempool_alloc(mempool *mempool, size_t size);
void mempool_free(mempool *mempool);

#ifdef INCLUDE_UNIT_TESTS
void mempool_unit_tests();
#endif

