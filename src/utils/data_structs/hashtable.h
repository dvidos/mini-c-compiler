#pragma once
#include <stdbool.h>
#include "../mempool.h"
#include "../unit_tests.h"
#include "../data_types/str.h"
#include "iterator.h"


typedef struct hashtable hashtable;

hashtable *new_hashtable(mempool *mp, int capacity);
int   hashtable_length(hashtable *h);
bool  hashtable_is_empty(hashtable *h);
void  hashtable_set(hashtable *h, str *key, void *data); // O(1)
void *hashtable_get(hashtable *h, str *key); // O(1)
bool  hashtable_delete(hashtable *h, str *key); // O(1)
bool  hashtable_contains(hashtable *h, str *key); // O(1)
void  hashtable_clear(hashtable *h);
iterator *hashtable_create_iterator(hashtable *h, mempool *m);

#ifdef INCLUDE_UNIT_TESTS
void hashtable_unit_tests();
#endif
