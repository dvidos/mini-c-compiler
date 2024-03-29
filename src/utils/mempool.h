#pragma once
#include "unit_tests.h"
#include <stddef.h>
#include <stdio.h>



// define this to enable tracking of memory allocations
#define MEMPOOL_TRACK_ALLOCATIONS    1

#ifdef MEMPOOL_TRACK_ALLOCATIONS
    // intention describes intended usage, for tracking purposes
    #define mpalloc(mempool, structure)         __mempool_alloc(mempool, sizeof(structure), #structure, __FILE__, __LINE__)
    #define mpallocn(mempool, bytes, descrip)   __mempool_alloc(mempool, bytes, #descrip ", " #bytes " bytes", __FILE__, __LINE__)
#else
    // intention, file, line are ignored
    #define mpalloc(mempool, structure)         __mempool_alloc(mempool, sizeof(structure), NULL, NULL, NULL)
    #define mpallocn(mempool, bytes, descrip)   __mempool_alloc(mempool, bytes,             NULL, NULL, NULL)
#endif


// a memory pool (mempool for short) allows for the definition of objects "lifetime"
// any objects can be allocated from it, and all objects are free'd together at the end
// it grows indefinitely as demand grows
typedef struct mempool mempool;

mempool *new_mempool();

// prefer the mpalloc() macro, to pass in tracking information
void *__mempool_alloc(mempool *mempool, size_t size, char *intention, char *file, int line);

// frees all the memory allocated on the pool
void mempool_release(mempool *mempool);

#ifdef MEMPOOL_TRACK_ALLOCATIONS
void mempool_print_allocations(mempool *mempool, FILE *f);
#endif

#ifdef INCLUDE_UNIT_TESTS
void mempool_unit_tests();
#endif

