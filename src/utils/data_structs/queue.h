#pragma once
#include <stdbool.h>
#include "../mempool.h"

typedef struct queue queue;

queue *new_queue(mempool *mp);
int   queue_length(queue *q);
bool  queue_is_empty(queue *q);
void  queue_clear(queue *q);
void  queue_put(queue *q, void *item);
void *queue_peek(queue *q);
void *queue_get(queue *q);

#ifdef INCLUDE_UNIT_TESTS
void queue_unit_tests();
#endif
