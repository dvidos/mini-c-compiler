#include "mempool.h"
#include <stdlib.h>
#include <string.h>


#define INITIAL_MEM_POOL_CAPACITY  256  // e.g. a few strings, or 64 pointers

struct mem_bucket {
    void *buffer; // the memory we malloc'ed for this bucket
    size_t capacity; // total size of this bucket
    size_t allocated; // the first x bytes that have already be allocated
    struct mem_bucket *next;
};

struct mempool {
    struct mem_bucket *buckets; // latest bucket first
    size_t total_capacity;
    size_t total_allocated;
    size_t allocations_count;
};

mempool *new_mempool() {
    mempool *mp = malloc(sizeof(mempool));
    memset(mp, 0, sizeof(mempool));

    struct mem_bucket *bucket = malloc(sizeof(struct mem_bucket));
    memset(bucket, 0, sizeof(struct mem_bucket));
    bucket->capacity = INITIAL_MEM_POOL_CAPACITY;
    bucket->buffer = malloc(bucket->capacity);

    mp->buckets = bucket;
    mp->total_capacity = bucket->capacity;
    return mp;
}

void *mempool_alloc(mempool *mp, size_t size) {
    // we could store usage statistics here, such as file:line, purpose, type etc.
    void *ptr;

    // see if there is capacity in any bucket
    struct mem_bucket *bucket = mp->buckets;
    while (bucket != NULL) {
        if (bucket->allocated + size <= bucket->capacity) {
            ptr = bucket->buffer + bucket->allocated;
            bucket->allocated += size;
            mp->total_allocated += size;
            mp->allocations_count += 1;
            return ptr;
        }
        bucket = bucket->next;
    }

    // we exhausted all the buckets and did not find capacity.
    // allocate a new bucket with enough capacity for at least some elements
    size_t bucket_capacity = mp->buckets->capacity * 2;
    if (bucket_capacity < size * 4)
        bucket_capacity = size * 4;
    
    bucket = malloc(sizeof(struct mem_bucket));
    memset(bucket, 0, sizeof(struct mem_bucket));
    bucket->capacity = bucket_capacity;
    bucket->buffer = malloc(bucket->capacity);

    // insert first in the chain
    bucket->next = mp->buckets;
    mp->buckets = bucket;
    mp->total_capacity += bucket->capacity;

    // also allocate
    ptr = bucket->buffer;
    bucket->allocated += size;
    mp->total_allocated += size;
    mp->allocations_count += 1;

    return ptr;
}

void mempool_free(mempool *mp) {
    struct mem_bucket *next;
    struct mem_bucket *bucket = mp->buckets;
    while (bucket != NULL) {  
        next = bucket->next;
        free(bucket->buffer);
        free(bucket);
        bucket = next;
    }

    free(mp);
}

#ifdef INCLUDE_UNIT_TESTS
void mempool_unit_tests() {

    // initial conditions
    mempool *mp = new_mempool();
    assert(mp != NULL);
    assert(mp->total_capacity > 0);
    assert(mp->total_allocated == 0);
    assert(mp->allocations_count == 0);
    assert(mp->buckets != NULL);
    assert(mp->buckets->capacity > 0);
    assert(mp->buckets->allocated == 0);
    assert(mp->buckets->buffer != NULL);
    assert(mp->buckets->next == NULL);

    // small allocation
    void *ptr = mempool_alloc(mp, 64);
    assert(ptr != NULL);
    assert(mp->allocations_count == 1);
    assert(mp->total_allocated == 64);
    assert(ptr == mp->buckets->buffer);
    assert(mp->buckets->allocated == 64);

    // check large allocation requires grabbing a new segment
    void *ptr2 = mempool_alloc(mp, 64 * 1024);
    assert(ptr2 != NULL);
    assert(mp->buckets->next != NULL);
    assert(ptr2 == mp->buckets->buffer); // the new segment was inserted at head

    // freeing, just to check for segfault
    mempool_free(mp);


    // large scale experiment: 32K pointers of 64K = 2GB
    int max_pointers = 32 * 1014;
    size_t chunk_size = 64 * 1024;
    mp = new_mempool();

    // first we need a chunk to hold all the pointers
    void **arr = (void **)mempool_alloc(mp, sizeof(void *) * max_pointers); // should be 40K or 80K
    for (int i = 0; i < max_pointers; i++) {
        arr[i] = mempool_alloc(mp, chunk_size);
    }

    // would be interesting to print state here
    // looking at debugger, total_capacity 4251986176, total_allocated 2126771712, allocations_count: 32449
    // 13 total buckets
    assert(mp->allocations_count == max_pointers + 1);
    assert(mp->total_allocated == (sizeof(void *) * max_pointers) + (max_pointers * chunk_size));
    mempool_free(mp);
}
#endif