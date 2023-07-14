#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "queue.h"


typedef struct queue_node {
    void *item;
    struct queue_node *next;
} queue_node;

struct queue {
    int items_count;
    queue_node *head;
    queue_node *tail;
    mempool *mempool;
};

queue *new_queue(mempool *mp) {
    queue *q = mpalloc(mp, queue);
    q->mempool = mp;
    return q;
}

int queue_length(queue *q) {
    return q->items_count;
}

bool queue_is_empty(queue *q) {
    return q->items_count == 0;
}

void queue_clear(queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->items_count = 0;
}

void queue_put(queue *q, void *item) {
    queue_node *n = mpalloc(q->mempool, queue_node);
    n->item = item;
    n->next = NULL;

    if (q->head == NULL) {
        q->head = n;
        q->tail = n;
        q->items_count = 1;
    } else {
        q->tail->next = n;
        q->tail = n;
        q->items_count++;
    }
}

void *queue_peek(queue *q) {
    return q->head == NULL ? NULL : q->head->item;
}

void *queue_get(queue *q) {
    if (q->head == NULL)
        return NULL;

    queue_node *n = q->head;  // what a joy to not have to free() !!!
    if (n == q->tail) {
        // was the only item
        q->head = NULL;
        q->tail = NULL;
        q->items_count = 0;
    } else {
        q->head = q->head->next;
        q->items_count--;
    }

    return n->item; 
}

#ifdef INCLUDE_UNIT_TESTS
void queue_unit_tests() {
    mempool *mp = new_mempool();

    // create
    queue *q = new_queue(mp);
    assert(q != NULL);

    // assert empty, length, get returns NULL
    assert(queue_is_empty(q));
    assert(queue_length(q) == 0);
    assert(queue_peek(q) == NULL);

    // push two things
    char c1 = 'a';
    char c2 = 'b';
    queue_put(q, &c1);
    queue_put(q, &c2);

    // assert non empty, length
    assert(!queue_is_empty(q));

    // assert peek, then get one, peek, then get the other
    assert(queue_length(q) == 2);
    assert(queue_peek(q) == &c1);
    assert(queue_get(q) == &c1);

    assert(queue_length(q) == 1);
    assert(queue_peek(q) == &c2);
    assert(queue_get(q) == &c2);
    
    assert(queue_length(q) == 0);
    assert(queue_peek(q) == NULL);
    assert(queue_is_empty(q));

    // add, clear
    queue_put(q, &c1);
    assert(queue_length(q) == 1);
    queue_clear(q);
    assert(queue_length(q) == 0);


    mempool_release(mp);
}
#endif

