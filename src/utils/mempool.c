#include "mempool.h"
#include <stdlib.h>
#include <string.h>


#define INITIAL_MEM_POOL_CAPACITY  256  // e.g. a few strings, or 64 pointers

#ifdef MEMPOOL_TRACK_ALLOCATIONS
    struct allocation_info {
        size_t size;
        char *intention;
        char *file;
        int line;
    };
    #define ALLOCATION_INFO_SIZE   sizeof(struct allocation_info)
#else
    #define ALLOCATION_INFO_SIZE   0
#endif


struct mem_bucket {
    void *buffer; // the memory we malloc'ed for this bucket
    size_t capacity; // total size of this bucket
    size_t allocated; // the first x bytes that have already be allocated (includes tracking info)
    size_t allocations_count;
    struct mem_bucket *next;
};

struct mempool {
    struct mem_bucket *buckets; // latest bucket first
    int    num_buckets;
    size_t total_capacity;
    size_t allocations_count;
    size_t total_allocated;     // (includes tracking info)
};

mempool *new_mempool() {
    mempool *mp = malloc(sizeof(mempool));
    memset(mp, 0, sizeof(mempool));

    struct mem_bucket *bucket = malloc(sizeof(struct mem_bucket));
    memset(bucket, 0, sizeof(struct mem_bucket));
    bucket->capacity = INITIAL_MEM_POOL_CAPACITY;
    bucket->buffer = malloc(bucket->capacity);
    memset(bucket->buffer, 0, bucket->capacity); // ensure we shall be handing off clean buffers

    mp->buckets = bucket;
    mp->num_buckets = 1;
    mp->total_capacity = bucket->capacity;
    return mp;
}

// calculate, create and insert a new bucket
static inline struct mem_bucket *mempool_add_new_bucket(mempool *mp, size_t alloc_size) {

    // make it double the previous one, but allow it to fit at least 4 items
    size_t capacity = mp->buckets->capacity * 2;
    if (capacity < (alloc_size + MEMPOOL_TRACK_ALLOCATIONS) * 4)
        capacity = (alloc_size + MEMPOOL_TRACK_ALLOCATIONS) * 4;

    struct mem_bucket *bucket = malloc(sizeof(struct mem_bucket));
    memset(bucket, 0, sizeof(struct mem_bucket));
    bucket->capacity = capacity;
    bucket->buffer = malloc(bucket->capacity);
    memset(bucket->buffer, 0, bucket->capacity); // ensure we shall be handing off clean buffers

    // insert at start, to make this the first candidate for allocations
    bucket->next = mp->buckets;
    mp->buckets = bucket;
    mp->num_buckets += 1;
    mp->total_capacity += bucket->capacity;

    return bucket;
}

// if does not fit in bucket, return NULL
static inline void *mempool_alloc_in_bucket(mempool *mp, struct mem_bucket *b, size_t size, char *intent, char *file, int line) {

    // if we don't fit, we fail, a new bucket will be created
    if (b->allocated + ALLOCATION_INFO_SIZE + size > b->capacity)
        return NULL;

    // the new ptr will be here
    void *ptr = b->buffer + b->allocated + ALLOCATION_INFO_SIZE;

    // fill in information, if enabled
    #ifdef MEMPOOL_TRACK_ALLOCATIONS
        struct allocation_info *ai = (struct allocation_info *)(ptr - ALLOCATION_INFO_SIZE);
        ai->size = size;
        ai->intention = intent;
        ai->file = file;
        ai->line = line;
    #endif

    // housekeeping
    b->allocations_count += 1;
    b->allocated += size + ALLOCATION_INFO_SIZE;
    mp->allocations_count += 1;
    mp->total_allocated += size + ALLOCATION_INFO_SIZE;

    memset(ptr, 0, size);
    return ptr;
}

void *__mempool_alloc(mempool *mp, size_t size, char *intent, char *file, int line) {
    void *ptr;

    // see if there is capacity in any bucket
    struct mem_bucket *bucket = mp->buckets;
    while (bucket != NULL) {
        ptr = mempool_alloc_in_bucket(mp, bucket, size, intent, file, line);
        if (ptr != NULL)
            return ptr;
        bucket = bucket->next;
    }

    // we exhausted all the buckets and did not find capacity.
    // allocate a new bucket with enough capacity for at least some elements
    bucket = mempool_add_new_bucket(mp, size);
    ptr = mempool_alloc_in_bucket(mp, bucket, size, intent, file, line);

    return ptr;
}

void mempool_release(mempool *mp) {
    struct mem_bucket *next;
    struct mem_bucket *bucket = mp->buckets;
    while (bucket != NULL) {  
        next = bucket->next;
        memset(bucket->buffer, 0, bucket->capacity);
        free(bucket->buffer);
        memset(bucket, 0, sizeof(struct mem_bucket));
        free(bucket);
        bucket = next;
    }

    memset(mp, 0, sizeof(mempool));
    free(mp);
}


#ifdef MEMPOOL_TRACK_ALLOCATIONS
static void mempool_print_buckets_summary_inversed(mempool *mp, struct mem_bucket *b, int bucket_no, FILE *f) {
    if (mp == NULL && b == NULL) {
        fprintf(f, "  Bucket          Buffer   Capacity Allocations  Allocated  Remaining  Free%%\n");
        //         "     123  0x123456789012 1234567890  1234567890 1234567890 1234567890   123%"
        return;
    }

    if (b->next != NULL)
        mempool_print_buckets_summary_inversed(mp, b->next, bucket_no - 1, f);
    
    fprintf(f, "     %3d  %12p %10ld  %10ld %10ld %10ld   %3d%%\n", 
            bucket_no, b->buffer, b->capacity, b->allocations_count, b->allocated,
            (b->capacity - b->allocated),
            (int)(b->capacity == 0 ? 0 : ((b->capacity - b->allocated) * 100 / b->capacity)));
}

static void mempool_print_buckets_allocations_inversed(mempool *mp, struct mem_bucket *b, int bucket_no, FILE *f) {
    if (mp == NULL && b == NULL) {
        fprintf(f, "  Bucket         Address       Size  Intention             File, Line\n");
        //         "     123  0x123456789012 1234567890  12345678901234567890  1234567890123456..."
        return;
    }

    if (b->next != NULL)
        mempool_print_buckets_allocations_inversed(mp, b->next, bucket_no - 1, f);

    int offset = 0;
    while (offset < b->allocated) {
        struct allocation_info *ai = (struct allocation_info *)(b->buffer + offset);
        fprintf(f, "     %3d  %12p %10ld  %-20s  %s:%d\n", bucket_no, 
            ((void *)ai) + ALLOCATION_INFO_SIZE,
            ai->size, 
            ai->intention, ai->file, ai->line);
        offset += sizeof(struct allocation_info) + ai->size;
    }
}

void mempool_print_allocations(mempool *mempool, FILE *f) {
    // first, an intro
    fprintf(f, "Mem Pool, %d buckets, %ld bytes capacity, %ld bytes allocated, %ld allocations\n",
        mempool->num_buckets, mempool->total_capacity, mempool->total_allocated, mempool->allocations_count);

    // then print all buckets,
    mempool_print_buckets_summary_inversed(NULL, NULL, 0, f); // header
    mempool_print_buckets_summary_inversed(mempool, mempool->buckets, mempool->num_buckets, f);

    // then print all allocations
    mempool_print_buckets_allocations_inversed(NULL, NULL, 0, f); // header
    mempool_print_buckets_allocations_inversed(mempool, mempool->buckets, mempool->num_buckets, f);
}

#endif


#ifdef INCLUDE_UNIT_TESTS
void mempool_unit_tests() {

    // initial conditions
    mempool *mp = new_mempool();
    assert(mp != NULL);
    assert(mp->total_capacity > 0);
    assert(mp->total_allocated == 0);
    assert(mp->allocations_count == 0);
    assert(mp->num_buckets == 1);
    assert(mp->buckets != NULL);
    assert(mp->buckets->capacity > 0);
    assert(mp->buckets->allocated == 0);
    assert(mp->buckets->buffer != NULL);
    assert(mp->buckets->next == NULL);

    // small allocation
    char *ptr = mpallocn(mp, 64, "small alloc");
    assert(ptr != NULL);
    assert(mp->allocations_count == 1);
    assert(mp->total_allocated == 64 + ALLOCATION_INFO_SIZE);
    assert(ptr == mp->buckets->buffer + ALLOCATION_INFO_SIZE);
    assert(mp->buckets->allocated == 64 + ALLOCATION_INFO_SIZE);

    char sum = 0;
    for (int i = 0; i < 64; i++)
        sum += ptr[i];
    assert(sum == 0); // ensure the chunk delivered is clean

    // check large allocation requires grabbing a new segment
    char *ptr2 = mpallocn(mp, 64 * 1024, "large alloc");
    assert(ptr2 != NULL);
    assert(mp->buckets->next != NULL);
    assert(ptr2 == mp->buckets->buffer + ALLOCATION_INFO_SIZE); // the new segment was inserted at head

    sum = 0;
    for (int i = 0; i < 64 * 1024; i++)
        sum += ptr2[i];
    assert(sum == 0); // ensure the chunk delivered is clean

    // mempool_print_allocations(mp, stdout);
    /*
     |  Bucket          Buffer   Capacity Allocations  Allocated  Remaining  Free%
     |       1  0x555555581710        256           1         96        160    62%
     |       2  0x7ffff7f5e010     262148           1      65568     196580    74%
     |  Bucket         Address       Size  Intention             File, Line
     |       1  0x555555581730         64  small alloc           utils/mempool.c:208
     |       2  0x7ffff7f5e030      65536  large alloc           utils/mempool.c:216
    */

    // freeing, just to check for segfault
    mempool_release(mp);

    // large scale experiment: 32K pointers of 64K = 2GB
    int max_pointers = 32 * 10; // * 1024;
    size_t chunk_size = 64 * 1024 * 10;
    mp = new_mempool();

    // first we need a chunk to hold all the pointers
    void **arr = (void **)mpallocn(mp, sizeof(void *) * max_pointers, "pointers"); // should be 40K or 80K
    for (int i = 0; i < max_pointers; i++) {
        arr[i] = mpallocn(mp, chunk_size, "chunk");
    }

    // mempool_print_allocations(mp, stdout);

    // would be interesting to print state here
    // looking at debugger, total_capacity 4251986176, total_allocated 2126771712, allocations_count: 32449
    // 13 total buckets
    assert(mp->allocations_count == max_pointers + 1);
    assert(mp->total_allocated == 
        (sizeof(void *) * max_pointers + ALLOCATION_INFO_SIZE) + (max_pointers * (chunk_size + ALLOCATION_INFO_SIZE)));
    mempool_release(mp);
}
#endif