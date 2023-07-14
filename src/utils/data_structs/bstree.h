#pragma once
#include <stdbool.h>
#include "../mempool.h"
#include "../unit_tests.h"
#include "../data_types/str.h"
#include "iterator.h"


typedef struct bstree bstree;

bstree *new_bstree(mempool *mp);
int   bstree_length(bstree *t);
bool  bstree_empty(bstree *t);
void *bstree_get(bstree *t, str *key);
bool  bstree_put(bstree *t, str *key, void *data);
bool  bstree_delete(bstree *t, str *key);
void  bstree_clear(bstree *t);
bool  bstree_contains(bstree *t, str *key);
iterator *bstree_create_iterator(bstree *t, mempool *m);
void bstree_print(bstree *t, FILE *f);

#ifdef INCLUDE_UNIT_TESTS
void bstree_unit_tests();
#endif
