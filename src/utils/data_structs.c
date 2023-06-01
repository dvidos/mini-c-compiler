#include <string.h>
#include <stdlib.h>
#include "unit_tests.h"
#include "data_structs.h"
#include "mempool.h"

// ------------------------------------------------

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
    queue *q = mempool_alloc(mp, sizeof(queue), "queue");
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
    queue_node *n = mempool_alloc(q->mempool, sizeof(queue_node), "queue_node");
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


// -------------------------------------------

typedef struct stack_node {
    void *item;
    struct stack_node *next;
} stack_node;

struct stack {
    int items_count;
    stack_node *top; // where we push and pop
    mempool *mempool;
};

stack *new_stack(mempool *mp) {
    stack *s = mempool_alloc(mp, sizeof(stack), "stack");
    s->mempool = mp;
    return s;
}

int stack_length(stack *s) {
    return s->items_count;
}

bool stack_is_empty(stack *s) {
    return s->items_count == 0;
}

void stack_clear(stack *s) {
    s->top = NULL;
    s->items_count = 0;
}

void stack_push(stack *s, void *item) {
    stack_node *n = mempool_alloc(s->mempool, sizeof(stack_node), "stack_node");
    n->item = item;
    n->next = NULL;

    if (s->top == NULL) {
        s->top = n;
        s->items_count = 1;
    } else {
        n->next = s->top;
        s->top = n;
        s->items_count++;
    }
}

void *stack_peek(stack *s) {
    return s->top == NULL ? NULL : s->top->item;
}

void *stack_pop(stack *s) {
    if (s->top == NULL)
        return NULL;

    stack_node *n = s->top;  // what a joy to not have to free() !!!
    s->top = s->top->next;
    s->items_count--;

    return n->item; 
}

#ifdef INCLUDE_UNIT_TESTS
void stack_unit_tests() {
    mempool *mp = new_mempool();

    // create
    stack *s = new_stack(mp);
    assert(s != NULL);

    // assert empty, length, get returns NULL
    assert(stack_is_empty(s));
    assert(stack_length(s) == 0);
    assert(stack_peek(s) == NULL);

    // push two things
    char c1 = 'a';
    char c2 = 'b';
    stack_push(s, &c1);
    stack_push(s, &c2);

    // assert non empty, length
    assert(!stack_is_empty(s));

    // assert peek, then get one, peek, then get the other
    assert(stack_length(s) == 2);
    assert(stack_peek(s) == &c2);
    assert(stack_pop(s) == &c2);

    assert(stack_length(s) == 1);
    assert(stack_peek(s) == &c1);
    assert(stack_pop(s) == &c1);
    
    assert(stack_length(s) == 0);
    assert(stack_peek(s) == NULL);
    assert(stack_is_empty(s));

    // add, clear
    stack_push(s, &c1);
    assert(stack_length(s) == 1);
    stack_clear(s);
    assert(stack_length(s) == 0);

    mempool_release(mp);
}
#endif

// -------------------------------------------

typedef struct llist_node {
    void *data;
    struct llist_node *next;
    struct llist_node *prev;
} llist_node;

struct llist {
    int items_count;
    struct llist_node *head;
    struct llist_node *tail;
    mempool *mempool;
};

llist *new_llist(mempool *mp) {
    llist *l = mempool_alloc(mp, sizeof(llist), "llist");
    memset(l, 0, sizeof(llist));
    l->mempool = mp;
    return l;
}

int llist_length(llist *l) {
    return l->items_count;
}

bool llist_is_empty(llist *l) {
    return l->items_count == 0;
}

void *llist_get(llist *l, int index) {
    llist_node *n = l->head;
    while (index-- > 0 && n != NULL)
        n = n->next;
    
    return n == NULL ? NULL : n->data;
}

void llist_add(llist *l, void *item) {
    llist_node *n = mempool_alloc(l->mempool, sizeof(llist_node), "llist_node");
    memset(n, 0, sizeof(llist_node));
    n->data = item;

    if (l->tail == NULL) {
        l->head = n;
        l->tail = n;
        l->items_count = 1;
    } else {
        n->prev = l->tail;
        l->tail->next = n;
        l->tail = n;
        l->items_count++;
    }
}

bool llist_contains(llist *l, void *item) {
    llist_node *n = l->head;
    while (n != NULL) {
        if (n->data == item)
            return true;
        n = n->next;
    }
    return false;
}

int llist_index_of(llist *l, void *item) {
    llist_node *n = l->head;
    int index = 0;
    while (n != NULL) {
        if (n->data == item)
            return index;
        n = n->next;
        index++;
    }
    return -1;
}

bool llist_insert_at(llist *l, int index, void *item) {
    if (index < 0 || index > l->items_count) {
        // invalid location
        return false;

    } else if (index == l->items_count) {
        llist_add(l, item);
        return true;

    } else if (index == 0) {
        // index at the start
        llist_node *new_node = mempool_alloc(l->mempool, sizeof(llist_node), "llist_node");
        memset(new_node, 0, sizeof(llist_node));
        new_node->data = item;
        new_node->next = l->head;
        l->head = new_node;
        if (new_node->next != NULL)
            new_node->next->prev = new_node;
        l->items_count++;

    } else {
        // index in the middle
        llist_node *new_node = mempool_alloc(l->mempool, sizeof(llist_node), "llist_node");
        memset(new_node, 0, sizeof(llist_node));
        new_node->data = item;
        new_node->next = l->head;
        llist_node *n = l->head;
        while (index-- > 0)
            n = n->next;
        if (n == NULL)
            return false; // should not happen
        // insert right before n
        new_node->next = n;
        new_node->prev = n->prev;
        n->prev->next = new_node;
        n->prev = new_node;
        l->items_count++;
    }
}

bool llist_remove_at(llist *l, int index) {
    if (l->items_count == 0 || index < 0 || index >= l->items_count) {
        // nothing to remove, or out of bounds
        return false;

    } else if (l->items_count == 1 && index == 0) {
        // nice not to worry about freeing the node!
        l->head = NULL;
        l->tail = NULL;
        l->items_count = 0;

    } else if (index == 0) {
        // remove from head, there's at least two items
        l->head = l->head->next;
        l->head->prev = NULL;
        l->items_count--;

    } else if (index == l->items_count - 1) {
        // remove from tail, there's at least two items
        l->tail = l->tail->prev;
        l->tail->next = NULL;
        l->items_count--;

    } else { 
        // remove from the middle (not head, nor tail)
        llist_node *n = l->head;
        while (index-- > 0 && n != NULL)
            n = n->next;
        if (n == NULL)
            return false; // should not happen
        
        // n points to the node to-be-deleted
        n->prev->next = n->next;
        n->next->prev = n->prev;
        l->items_count--;
    }

    return true;
}

void llist_clear(llist *l) {
    l->head = NULL;
    l->tail = NULL;
    l->items_count = 0;
}

llist *llist_reverse(llist *l) {
    llist *new_list = mempool_alloc(l->mempool, sizeof(llist), "llist");
    memset(new_list, 0, sizeof(llist));
    new_list->mempool = l->mempool;
    llist_node *n = l->tail;
    while (n != NULL) {
        llist_add(new_list, n->data);
        n = n->prev;
    }
    return new_list;
}

static comparator_function *user_sort_comparator;
static int llist_sort_comparator(const void *a, const void *b) {
    // qsort passes pointers to the data to be compared.
    // but out data are already pointers to the user data.
    return user_sort_comparator(*(const void **)a, *(const void **)b);
}

int llist_find_first(llist *l, comparator_function *compare, void *item) {
    llist_node *n = l->head;
    int index = 0;
    while (n != NULL) {
        if (compare(n->data, item) == 0)
            return index;
        n = n->next;
        index++;
    }
    return -1;
}

int llist_find_last(llist *l, comparator_function *compare, void *item) {
    llist_node *n = l->tail;
    int index = l->items_count - 1;
    while (n != NULL) {
        if (compare(n->data, item) == 0)
            return index;
        n = n->prev;
        index--;
    }
    return -1;
}

llist *llist_sort(llist *l, comparator_function *compare, mempool *mp) {
    // allocate an array with all the pointers, then sort them, then create a new list

    mempool *scratch = new_mempool();
    void **ptr_array = mempool_alloc(scratch, l->items_count * sizeof(void *), "llist_ptr_array");
    llist_node *n = l->head;
    int i = 0;
    while (n != NULL) {
        ptr_array[i++] = n->data;
        n = n->next;
    }

    // qsort() will pass a pointer to the pointer of the actual data
    // so, we need an internal function, and to know which comparer to call.
    user_sort_comparator = compare;
    qsort(ptr_array, l->items_count, sizeof(void *), llist_sort_comparator);

    // now, create a new list with the sorted data
    llist *new_list = new_llist(mp);
    for (int i = 0; i < l->items_count; i++)
        llist_add(new_list, ptr_array[i]);

    // we can free the scratch pool
    mempool_release(scratch);
    return new_list;
}

llist *llist_unique(llist *l, comparator_function *compare, mempool *mp) {
    // simple approach for now, O(n^2 / 2)
    llist *new_list = new_llist(mp);
    llist_node *n = l->head;
    while (n != NULL) {
        
        if (llist_find_first(new_list, compare, n->data) == -1)
            llist_add(new_list, n->data);
        n = n->next;
    }

    return new_list;
}

llist *llist_filter(llist *l, filterer_function *filter, mempool *mp) {
    llist *new_list = mempool_alloc(mp, sizeof(llist), "llist");
    memset(new_list, 0, sizeof(llist));
    new_list->mempool = mp;

    llist_node *n = l->head;
    while (n != NULL) {
        if (filter(n->data))
            llist_add(new_list, n->data);
        n = n->next;
    }
    return new_list;
}

llist *llist_map(llist *l, mapper_function *map, mempool *mp) {
    llist *new_list = mempool_alloc(l->mempool, sizeof(llist), "llist");
    memset(new_list, 0, sizeof(llist));
    new_list->mempool = l->mempool;

    llist_node *n = l->head;
    while (n != NULL) {
        void *new_data = map(n->data, l->mempool);
        llist_add(new_list, new_data);
        n = n->next;
    }
    return new_list;
}

void *llist_reduce(llist *l, reducer_function *reduce, void *initial_value, mempool *mp) {
    void *p = initial_value;
    llist_node *n = l->head;
    while (n != NULL) {
        p = reduce(n->data, p, mp);
        n = n->next;
    }
    return p;
}

typedef struct llist_iterator_private_state {
    llist *list;
    llist_node *curr;
} llist_iterator_private_state;

static void *llist_iterator_reset(iterator *it) {
    llist_iterator_private_state *it_state = (llist_iterator_private_state *)it->private_data;
    it_state->curr = it_state->list->head;
    return it_state->curr == NULL ? NULL : it_state->curr->data;
}

static bool llist_iterator_valid(iterator *it) {
    llist_iterator_private_state *it_state = (llist_iterator_private_state *)it->private_data;
    return it_state->curr != NULL;
}

static void *llist_iterator_next(iterator *it) {
    llist_iterator_private_state *it_state = (llist_iterator_private_state *)it->private_data;
    it_state->curr = it_state->curr->next;
    return it_state->curr == NULL ? NULL : it_state->curr->data;
}

iterator *llist_create_iterator(llist *l, mempool *mp) {
    llist_iterator_private_state *it_state = mempool_alloc(mp, sizeof(llist_iterator_private_state), "llist_iterator_private_state");
    memset(it_state, 0, sizeof(llist_iterator_private_state));
    it_state->list = l;

    iterator *it = mempool_alloc(mp, sizeof(iterator), "iterator");
    it->private_data = it_state;
    it->reset = llist_iterator_reset;
    it->valid = llist_iterator_valid;
    it->next = llist_iterator_next;
    return it;
}

hashtable *llist_group(llist *l, classifier_function *classify, mempool *mp) {
    // we can derive groups, then create a hashtable with lists of the contents of each group.
    return NULL;
}

#ifdef INCLUDE_UNIT_TESTS
static bool __llist_unit_test_filter(void *item) {
    return strcmp((char *)item, "A") == 0;
}
static void *__llist_unit_test_map(void *item, mempool *mp) {
    char *src = (char *)item;
    char *dst = mempool_alloc(mp, strlen(src) * 2 + 4, "string mapping");
    strcpy(dst, src);
    strcat(dst, "-");
    strcat(dst, src);
    return dst;
}
static void *__llist_unit_test_reduce(void *item, void *prev_value, mempool *mp) {
    char *reduced = mempool_alloc(mp, strlen((char *)prev_value) + 1 + strlen((char *)item) + 1, "reduced value");

    strcpy(reduced, (char *)prev_value);
    if (strlen(reduced) > 0)
        strcat(reduced, ",");
    strcat(reduced, (char *)item);

    return reduced;
}

void llist_unit_tests() {
    mempool *mp = new_mempool();
    char *s;

    void *a = mempool_alloc(mp, 4, "dummy data");
    void *b = mempool_alloc(mp, 4, "dummy data");
    void *c = mempool_alloc(mp, 4, "dummy data");
    strcpy(a, "A");
    strcpy(b, "B");
    strcpy(c, "C");

    // test initial conditions
    llist *l = new_llist(mp);
    assert(l != NULL);
    assert(l->items_count == 0);
    assert(l->head == NULL);
    assert(l->tail == NULL);
    assert(llist_length(l) == 0);
    assert(llist_is_empty(l));
    assert(llist_get(l, 0) == NULL);
    assert(!llist_contains(l, a));

    // adding, getting
    llist_add(l, a);
    llist_add(l, b);
    llist_add(l, c);
    assert(l->items_count == 3);
    assert(l->head != NULL && l->head->data == a);
    assert(l->tail != NULL && l->tail->data == c);
    assert(llist_get(l, 0) == a);
    assert(llist_get(l, 1) == b);
    assert(llist_get(l, 2) == c);

    // test clearing
    llist_clear(l);
    assert(l->items_count == 0);
    assert(l->head == NULL);
    assert(l->tail == NULL);

    // test contains
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_add(l, c);
    assert(llist_contains(l, b));
    llist_clear(l);
    assert(!llist_contains(l, b));

    // insert start
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_insert_at(l, 0, c);
    assert(llist_get(l, 0) == c);
    assert(llist_get(l, 1) == a);
    assert(llist_get(l, 2) == b);

    // insert middle
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_insert_at(l, 1, c);
    assert(llist_get(l, 0) == a);
    assert(llist_get(l, 1) == c);
    assert(llist_get(l, 2) == b);

    // insert at end
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_insert_at(l, 2, c);
    assert(llist_get(l, 0) == a);
    assert(llist_get(l, 1) == b);
    assert(llist_get(l, 2) == c);

    // delete from start
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_add(l, c);
    llist_remove_at(l, 0);
    assert(llist_get(l, 0) == b);
    assert(llist_get(l, 1) == c);

    // delete from middle
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_add(l, c);
    llist_remove_at(l, 1);
    assert(llist_get(l, 0) == a);
    assert(llist_get(l, 1) == c);

    // delete from end
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_add(l, c);
    llist_remove_at(l, 2);
    assert(llist_get(l, 0) == a);
    assert(llist_get(l, 1) == b);

    // delete the last one
    llist_clear(l);
    llist_add(l, a);
    llist_remove_at(l, 0);
    assert(llist_length(l) == 0);
    assert(l->head == NULL);
    assert(l->tail == NULL);

    // reverse
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_add(l, c);
    llist *reversed = llist_reverse(l);
    assert(llist_get(reversed, 0) == c);
    assert(llist_get(reversed, 1) == b);
    assert(llist_get(reversed, 2) == a);

    // find first + find last
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_add(l, a);
    assert(llist_find_first(l, (comparator_function *)strcmp, a) == 0);
    assert(llist_find_first(l, (comparator_function *)strcmp, c) == -1);
    assert(llist_find_last(l, (comparator_function *)strcmp, a) == 2);
    assert(llist_find_last(l, (comparator_function *)strcmp, c) == -1);

    // sort
    llist_clear(l);
    llist_add(l, b);
    llist_add(l, c);
    llist_add(l, a);
    llist *sorted = llist_sort(l, (comparator_function *)strcmp, mp);
    assert(llist_get(sorted, 0) == a);
    assert(llist_get(sorted, 1) == b);
    assert(llist_get(sorted, 2) == c);

    // unique
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_add(l, b);
    llist_add(l, a);
    llist_add(l, c);
    llist_add(l, a);
    assert(llist_length(l) == 6);
    llist *uniq = llist_unique(l, (comparator_function *)strcmp, mp);
    assert(llist_length(uniq) == 3);
    assert(llist_get(uniq, 0) == a);
    assert(llist_get(uniq, 1) == b);
    assert(llist_get(uniq, 2) == c);

    // filter
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_add(l, b);
    llist_add(l, a);
    llist_add(l, c);
    llist_add(l, a);
    assert(llist_length(l) == 6);
    llist *filtered = llist_filter(l, __llist_unit_test_filter, mp);
    assert(llist_length(filtered) == 3);
    assert(llist_get(filtered, 0) == a);
    assert(llist_get(filtered, 1) == a);
    assert(llist_get(filtered, 2) == a);

    // map
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_add(l, c);
    llist *mapped = llist_map(l, __llist_unit_test_map, mp);
    assert(llist_length(mapped) == 3);
    assert(strcmp(llist_get(mapped, 0), "A-A") == 0);
    assert(strcmp(llist_get(mapped, 1), "B-B") == 0);
    assert(strcmp(llist_get(mapped, 2), "C-C") == 0);

    // reduce
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_add(l, c);
    void *result = llist_reduce(l, __llist_unit_test_reduce, "", mp);
    assert(result != NULL);
    assert(strcmp(result, "A,B,C") == 0);

    // create_iterator
    llist_clear(l);
    llist_add(l, a);
    llist_add(l, b);
    llist_add(l, c);
    iterator *it = llist_create_iterator(l, mp);
    s = it->reset(it);
    assert(s != NULL);
    assert(it->valid(it));
    assert(strcmp(s, "A") == 0);
    s = it->next(it);
    assert(s != NULL);
    assert(it->valid(it));
    assert(strcmp(s, "B") == 0);
    s = it->next(it);
    assert(s != NULL);
    assert(it->valid(it));
    assert(strcmp(s, "C") == 0);
    s = it->next(it);
    assert(s == NULL);
    assert(!it->valid(it));

    // group


    //mempool_print_allocations(mp, stdout); // for fun
    mempool_release(mp);
}
#endif



// ------------------------------------------------------------------

typedef struct bstree_node {
    str *key;
    void *data;
    struct bstree_node *left;
    struct bstree_node *right;
    struct bstree_node *parent;
} bstree_node;

struct bstree {
    int nodes_count;
    bstree_node *root;
    mempool *mempool; // for allocating new nodes
};

bstree *new_bstree(mempool *mp) {
    bstree *t = mempool_alloc(mp, sizeof(bstree), "bstree");
    t->mempool = mp;
    return t;
}

int bstree_length(bstree *t) {
    return t->nodes_count;
}

bool bstree_empty(bstree *t) {
    return t->nodes_count == 0;
}

bool bstree_contains(bstree *t, str *key) {
    bstree_node *n = t->root;
    while (n != NULL) {
        int c = str_cmp(key, n->key);
        if      (c == 0) return true;
        else if (c  < 0) n = n->left;
        else if (c  > 0) n = n->right;
    }
    return false;
}

void *bstree_get(bstree *t, str *key) { // O(lg N) time complexity (1 million nodes -> 20 hops)
    bstree_node *n = t->root;
    while (n != NULL) {
        int c = str_cmp(key, n->key);
        if      (c == 0) return n->data;
        else if (c  < 0) n = n->left;
        else if (c  > 0) n = n->right;
    }
    return n;
}

bool bstree_put(bstree *t, str *key, void *data) {
    bstree_node *new_node = mempool_alloc(t->mempool, sizeof(bstree_node), "bstree_node");
    memset(new_node, 0, sizeof(bstree_node));
    new_node->key = key;
    new_node->data = data;

    if (t->root == NULL) {
        t->root = new_node;
        t->nodes_count++;
        return true;
    }

    bstree_node *n = t->root;
    while (n != NULL) {
        int c = str_cmp(key, n->key);
        if (c == 0)
            return false; // key already in tree
        
        if (c < 0) {
            if (n->left == NULL) {
                n->left = new_node;
                t->nodes_count++;
                return true;
            }
            n = n->left;
        } else if (c > 0) {
            if (n->right == NULL) {
                n->right = new_node;
                t->nodes_count++;
                return true;
            }
            n = n->right;
        }
    }

    // we should not get here
    return false;
}

static void bstree_remove_node(bstree_node *node, bstree_node **pointer_to_node) {
    if (node->left == NULL && node->right == NULL) {
        // node is a leaf, just remove it.
        *pointer_to_node = NULL;

    } else if (node->left != NULL && node->right == NULL) {
        // node only has left node, bring it up to parent
        *pointer_to_node = node->left;
        node->left = NULL;

    } else if (node->left == NULL && node->right != NULL) {
        // node only has right node, bring it up to parent
        *pointer_to_node = node->right;
        node->right = NULL;

    } else {
        // node has two child nodes.
        // from the right subtree, find the smallest key node. this is the successor
        // put its key & data on the node under deletion
        // and remove this min node from its parent (it may have at most one node)
        bstree_node **pointer_to_successor = &node->right;
        bstree_node *successor = node->right;
        while (successor->left != NULL) {
            pointer_to_successor = &successor->left;
            successor = successor->left;
        }
        // copy key & data to the node to be deleted
        node->key = successor->key;
        node->data = successor->data;
        // remove that successor
        bstree_remove_node(successor, pointer_to_successor);
    }
}

bool bstree_delete(bstree *t, str *key) {

    // keep a trailing pointer to the pointer that points the node to delete
    bstree_node *node = t->root;
    bstree_node **pointer_to_node = &t->root;
    while (node != NULL) {
        int c = str_cmp(key, node->key);
        if (c == 0) {
            bstree_remove_node(node, pointer_to_node);
            t->nodes_count--;
            return true;
        }
        else if (c < 0) {
            if (node->left == NULL)
                return false; // not found

            pointer_to_node = &node->left;
            node = node->left;
        }
        else if (c > 0) {
            if (node->right == NULL)
                return false; // not found

            pointer_to_node = &node->right;
            node = node->right;
        }
    }

    return false;
}

void bstree_clear(bstree *t) {
    t->root = NULL;
    t->nodes_count = 0;
}

typedef struct bstree_iterator_private_state {
    bstree *tree;
    stack *stack;  // stack holds bstree_nodes. the stack top is the iterator's current.
    bool last_operation_valid;
} bstree_iterator_private_state;

static void *bstree_iterator_reset(iterator *it) {
    bstree_iterator_private_state *it_state = (bstree_iterator_private_state *)it->private_data;

    stack_clear(it_state->stack);
    if (it_state->tree->root == NULL)
        return NULL;
    
    // find the smallest node
    bstree_node *n = it_state->tree->root;
    while (n != NULL) {
        stack_push(it_state->stack, n);
        n = n->left;
    }

    return ((bstree_node *)stack_peek(it_state->stack))->data;
}

static bool bstree_iterator_valid(iterator *it) {
    bstree_iterator_private_state *it_state = (bstree_iterator_private_state *)it->private_data;
    return stack_peek(it_state->stack) != NULL;
}

static void *bstree_iterator_next(iterator *it) {
    bstree_iterator_private_state *it_state = (bstree_iterator_private_state *)it->private_data;

    // if there was no node so far, there's nowhere to go from here
    bstree_node *curr = stack_peek(it_state->stack);
    if (curr == NULL)
        return NULL;
    
    // if there is a right child, go to the smallest key of the right subtree
    if (curr->right != NULL) {
        curr = curr->right;
        stack_push(it_state->stack, curr);
        while (curr->left != NULL) {
            curr = curr->left;
            stack_push(it_state->stack, curr);
        }
        return curr->data;
    }

    // as there is no right child, we need to move up the tree to find something larger.
    curr = stack_pop(it_state->stack);

    // if stack is empty, it means we popped the root, and there was no right child.
    if (stack_is_empty(it_state->stack)) {
        return NULL;
    }

    // if we are the left child of the parent, they are the next in order.
    bstree_node *parent = stack_peek(it_state->stack);
    if (curr == parent->left) {
        return parent->data;
    }

    // so we were the right child of the parent. we are done, they are done as well.  
    // we must go up, all the way until we find a parent whose we are the left child.
    // if we find no such parent, it means we were the rightmost node, hence the last one
    curr = stack_pop(it_state->stack);
    while (!stack_is_empty(it_state->stack)) {
        parent = stack_peek(it_state->stack);
        if (curr == parent->left) {
            // we were the left child of this parent, they are next in order.
            return parent->data;
        }
        // so we were the right child of this parent, we must continue up
        curr = stack_pop(it_state->stack);
    }
    
    // we emptied the stack. this means we don't have a parent to go up to.
    // we must have been the rightmost child of all. this is the end.
    return NULL;
}

iterator *bstree_create_iterator(bstree *t, mempool *mp) {
    bstree_iterator_private_state *it_state = mempool_alloc(mp, sizeof(bstree_iterator_private_state), "bstree_iterator_private_state");
    it_state->tree = t;
    it_state->stack = new_stack(mp);

    iterator *it = mempool_alloc(mp, sizeof(iterator), "iterator");
    it->private_data = it_state;
    it->reset = bstree_iterator_reset;
    it->valid = bstree_iterator_valid;
    it->next = bstree_iterator_next;
    return it;
}

static void bstree_print_node(bstree_node *n, int depth, char prefix, FILE *f) {
    if (n == NULL)
        return;
    fprintf(f, "%*s%c: %s\n", depth * 4, "", prefix, str_charptr(n->key));
    bstree_print_node(n->left, depth + 1, 'L', f);
    bstree_print_node(n->right, depth + 1, 'R', f);
}

void bstree_print(bstree *t, FILE *f) {
    bstree_print_node(t->root, 0, 'R', f);
}

static void bstree_compact_string_node(bstree_node *n, str *s) {
    if (n == NULL)
        return;

    if (n->left == NULL && n->right == NULL) {
        str_cat(s, n->key);
    } else {
        str_cats(s, "(");
        str_cat(s, n->key);
        str_cats(s, ",");
        bstree_compact_string_node(n->left, s);
        str_cats(s, ",");
        bstree_compact_string_node(n->right, s);
        str_cats(s, ")");
    }
}

str *bstree_compact_string(bstree *t, mempool *mp) {
    str *s = new_str(mp, "");
    bstree_compact_string_node(t->root, s);
    return s;
}

#ifdef INCLUDE_UNIT_TESTS
static bstree *bstree_create_unit_test_tree(mempool *mp) {
    /*
        Create the following tree, to be used in testing.
        
        |      E
        |     / \
        |    B   F
        |   / \   \
        |  A   D   G
        |     /
        |    C
    */
    bstree *t = new_bstree(mp);
    bstree_put(t, new_str(mp, "E"), "E");
    bstree_put(t, new_str(mp, "B"), "B");
    bstree_put(t, new_str(mp, "F"), "F");
    bstree_put(t, new_str(mp, "A"), "A");
    bstree_put(t, new_str(mp, "D"), "D");
    bstree_put(t, new_str(mp, "C"), "C");
    bstree_put(t, new_str(mp, "G"), "G");

    return t;
}

void bstree_unit_tests() {
    mempool *mp = new_mempool();

    str *key_a = new_str(mp, "key a");
    str *key_b = new_str(mp, "key b");
    str *key_c = new_str(mp, "key c");
    char *data_a = "data a";
    char *data_b = "data b";
    char *data_c = "data c";
    str *key_z = new_str(mp, "key z");
    void *p;

    bstree *t = new_bstree(mp);
    assert(bstree_length(t) == 0);
    assert(bstree_empty(t));

    // add root
    bstree_put(t, key_b, data_b);
    assert(bstree_length(t) == 1);
    assert(!bstree_empty(t));
    assert(strcmp(str_charptr(t->root->key), "key b") == 0);

    // simple addition, make sure left/right
    bstree_put(t, key_a, data_a);
    bstree_put(t, key_c, data_c);
    assert(bstree_length(t) == 3);
    assert(strcmp(str_charptr(t->root->left->key), "key a") == 0);
    assert(strcmp(str_charptr(t->root->right->key), "key c") == 0);
    assert(strcmp(t->root->left->data, "data a") == 0);
    assert(strcmp(t->root->right->data, "data c") == 0);
    assert(t->root->left->left == NULL);
    assert(t->root->left->right == NULL);
    assert(t->root->left->left == NULL);
    assert(t->root->right->right == NULL);
    
    // verify find / contains
    assert(bstree_contains(t, key_c));
    p = bstree_get(t, key_c);
    assert(p != NULL);
    assert(strcmp(p, "data c") == 0);
    assert(!bstree_contains(t, key_z));
    p = bstree_get(t, key_z);
    assert(p == NULL);


    // test deletion, case 1: delete a leaf node
    t = bstree_create_unit_test_tree(mp);
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,(D,C,)),(F,,G))") == 0);
    bstree_delete(t, new_str(mp, "C"));
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,D),(F,,G))") == 0);

    // test deletion, case 2: delete a node that has only left child
    t = bstree_create_unit_test_tree(mp);
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,(D,C,)),(F,,G))") == 0);
    bstree_delete(t, new_str(mp, "D"));
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,C),(F,,G))") == 0);

    // test deletion, case 3: delete a node that has only right child
    t = bstree_create_unit_test_tree(mp);
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,(D,C,)),(F,,G))") == 0);
    bstree_delete(t, new_str(mp, "F"));
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,(D,C,)),G)") == 0);

    // test deletion, case 4: delete a node that has two children
    t = bstree_create_unit_test_tree(mp);
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,(D,C,)),(F,,G))") == 0);
    bstree_delete(t, new_str(mp, "B"));
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(C,A,D),(F,,G))") == 0);

    // test deletion, case 4+: delete root node
    t = bstree_create_unit_test_tree(mp);
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(E,(B,A,(D,C,)),(F,,G))") == 0);
    bstree_delete(t, new_str(mp, "E"));
    assert(strcmp(str_charptr(bstree_compact_string(t, mp)), "(F,(B,A,(D,C,)),G)") == 0);


    // test iterator, traverse the tree, collect payloads
    t = bstree_create_unit_test_tree(mp);
    str *series = new_str(mp, "");
    iterator *it = bstree_create_iterator(t, mp);
    for_iterator(char, p, it)
        str_cats(series, p);
    assert(strcmp(str_charptr(series), "ABCDEFG") == 0);


    // in mem pool: 240 allocations, 12K bytes total, and not a single byte leaking!
    mempool_release(mp);
}
#endif


// --------------------------------------------------------

typedef struct hashtable_node {
    str *key;
    void *data;
    struct hashtable_node *next;
} hashtable_node;

struct hashtable {
    int capacity;
    int items_count;
    hashtable_node **items_arr;  // the array of node pointers
    mempool *mempool; // for creating nodes
};

hashtable *new_hashtable(mempool *mp, int capacity) {
    hashtable *h = mempool_alloc(mp, sizeof(hashtable), "hashtable");
    h->capacity = capacity;
    h->items_arr = (hashtable_node **)mempool_alloc(mp, capacity * sizeof(void *), "hashtable_items_arr");
    h->mempool = mp;
    return h;
}

int hashtable_length(hashtable *h) {
    return h->items_count;
}

bool hashtable_is_empty(hashtable *h) {
    return h->items_count == 0;
}

bool hashtable_contains(hashtable *h, str *key) { // O(1)
    int slot = (int)(str_hash(key) % h->capacity);
    hashtable_node *n = h->items_arr[slot];
    while (n != NULL) {
        if (str_cmp(n->key, key) == 0)
            return true;
        n = n->next;
    }
    return false;
}

void *hashtable_get(hashtable *h, str *key) { // O(1)
    int slot = (int)(str_hash(key) % h->capacity);
    hashtable_node *n = h->items_arr[slot];
    while (n != NULL) {
        if (str_cmp(n->key, key) == 0)
            return n->data;
        n = n->next;
    }
    return NULL;
}

static hashtable_node *hashtable_create_node(hashtable *h, str *key, void *data) {
    hashtable_node *node = mempool_alloc(h->mempool, sizeof(hashtable_node), "hashtable_node");
    node->key = key;
    node->data = data;
    node->next = NULL;
    return node;
}

void hashtable_set(hashtable *h, str *key, void *data) { // O(1)
    int slot = (int)(str_hash(key) % h->capacity);

    // easy solution if slot is empty
    if (h->items_arr[slot] == NULL) {
        h->items_arr[slot] = hashtable_create_node(h, key, data);
        h->items_count++;
        return;
    }

    // go over list, check if it exists, replace or append
    hashtable_node *curr = h->items_arr[slot];
    while (curr != NULL) {
        if (str_cmp(curr->key, key) == 0) {
            // we change data in place, without changing the items_count
            curr->data = data;
            return;
        }
        if (curr->next == NULL) {
            // this is the last item
            curr->next = hashtable_create_node(h, key, data);
            h->items_count++;
            return;
        }
        curr = curr->next;
    }
}

bool hashtable_delete(hashtable *h, str *key) { // O(1)
    int slot = (int)(str_hash(key) % h->capacity);
    if (h->items_arr[slot] == NULL)
        return false;

    // if it's the first entry of the chain, bypass it
    hashtable_node *n = h->items_arr[slot];
    if (str_cmp(n->key, key) == 0) {
        h->items_arr[slot] = n->next;
        h->items_count--;
        return true;
    }

    // walk the chain with a trailing pointer to remove the node.
    hashtable_node *trailing = n;
    n = n->next;
    while (n != NULL) {
        if (str_cmp(n->key, key) == 0) {
            trailing->next = n->next;
            h->items_count--;
            return true;
        }
        trailing = n;
        n = n->next;
    }

    return false;
}

void hashtable_clear(hashtable *h) {
    for (int i = 0; i < h->capacity; i++)
        h->items_arr[i] = NULL;
    h->items_count = 0;
}

typedef struct hashtable_iterator_private_state {
    hashtable *hashtable;
    int curr_slot_no;
    hashtable_node *curr_node;
} hashtable_iterator_private_state;

static void hashtable_iterator_advance(hashtable_iterator_private_state *it_state) {
    // maybe we are in a valid slot and there are more nodes in the chain
    if (it_state->curr_node != NULL && it_state->curr_node->next != NULL) {
        it_state->curr_node = it_state->curr_node->next;
        return;
    }

    // if we are already past the end, no point in continuing
    if (it_state->curr_slot_no >= it_state->hashtable->capacity) {
        it_state->curr_node = NULL;
        return;
    }

    // slot is somewhere in the array, but no more nodes in the chain.
    // find the start of the next valid chain.
    it_state->curr_slot_no++;
    while (it_state->hashtable->items_arr[it_state->curr_slot_no] == NULL 
        && it_state->curr_slot_no < it_state->hashtable->capacity)
        it_state->curr_slot_no += 1;
    
    // if we moved past end, there is no point
    if (it_state->curr_slot_no >= it_state->hashtable->capacity) {
        it_state->curr_node = NULL;
        return;
    }

    // we moved to a new chain.
    it_state->curr_node = it_state->hashtable->items_arr[it_state->curr_slot_no];
}

static void *hashtable_iterator_reset(iterator *it) {
    hashtable_iterator_private_state *it_state = (hashtable_iterator_private_state *)it->private_data;
    it_state->curr_slot_no = -1;
    it_state->curr_node = NULL;
    hashtable_iterator_advance(it_state);
    return it_state->curr_node == NULL ? NULL : it_state->curr_node->data;
}

static bool hashtable_iterator_valid(iterator *it) {
    hashtable_iterator_private_state *it_state = (hashtable_iterator_private_state *)it->private_data;
    return it_state->curr_node != NULL;
}

static void *hashtable_iterator_next(iterator *it) {
    hashtable_iterator_private_state *it_state = (hashtable_iterator_private_state *)it->private_data;
    hashtable_iterator_advance(it_state);
    return it_state->curr_node == NULL ? NULL : it_state->curr_node->data;
}

iterator *hashtable_create_iterator(hashtable *h, mempool *mp) {
    hashtable_iterator_private_state *it_state = mempool_alloc(mp, sizeof(hashtable_iterator_private_state), "hashtable_iterator_private_state");
    it_state->hashtable = h;
    it_state->curr_slot_no = -1;
    it_state->curr_node = NULL;

    iterator *it = mempool_alloc(mp, sizeof(iterator), "iterator");
    it->private_data = it_state;
    it->reset = hashtable_iterator_reset;
    it->valid = hashtable_iterator_valid;
    it->next = hashtable_iterator_next;
    return it;
}

#ifdef INCLUDE_UNIT_TESTS
void hashtable_unit_tests() {
    mempool *mp = new_mempool();

    // test initial conditions
    hashtable *h = new_hashtable(mp, 2);
    assert(hashtable_length(h) == 0);
    assert(hashtable_is_empty(h));

    str *key1 = new_str(mp, "key 1");
    str *key2 = new_str(mp, "key 2");
    str *key3 = new_str(mp, "key 3");
    char *payload1 = "payload 1";
    char *payload2 = "payload 2";
    char *payload3 = "payload 3";

    // test adding two items
    assert(!hashtable_contains(h, key1));
    hashtable_set(h, key1, payload1);
    hashtable_set(h, key2, payload2);
    assert(!hashtable_is_empty(h));
    assert(hashtable_length(h) == 2);
    assert(hashtable_contains(h, key1));
    assert(hashtable_contains(h, key2));

    // add an element that goes to the same chain as another
    hashtable_set(h, key3, payload3);
    assert(hashtable_length(h) == 3);
    assert(hashtable_contains(h, key1));
    assert(hashtable_contains(h, key2));
    assert(hashtable_contains(h, key3));

    // verify get
    assert(strcmp((char *)hashtable_get(h, key1), "payload 1") == 0);
    assert(strcmp((char *)hashtable_get(h, key2), "payload 2") == 0);
    assert(strcmp((char *)hashtable_get(h, key3), "payload 3") == 0);

    // test override value
    assert(strcmp((char *)hashtable_get(h, key1), "payload 1") == 0);
    hashtable_set(h, key1, "new payload 1");
    assert(hashtable_length(h) == 3);
    assert(strcmp((char *)hashtable_get(h, key1), "new payload 1") == 0);

    // test delete an item in a chain (and ensure the next in chain remains)
    // one of the keys is a chain, find which one, delete the chained item.
    // this depends on the hashing function, so hashing changes would break this test
    str *key_to_delete = NULL;
    str *key_to_keep = NULL;
    if (h->items_arr[0] != NULL && h->items_arr[0]->next != NULL) {
        key_to_delete = h->items_arr[0]->key;
        key_to_keep = h->items_arr[0]->next->key;
    } else if (h->items_arr[1] != NULL && h->items_arr[1]->next != NULL) {
        key_to_delete = h->items_arr[1]->key;
        key_to_keep = h->items_arr[1]->next->key;
    }
    hashtable_delete(h, key_to_delete);
    assert(hashtable_length(h) == 2);
    assert(!hashtable_contains(h, key_to_delete));
    assert(hashtable_contains(h, key_to_keep));
    assert(hashtable_get(h, key_to_delete) == NULL);
    assert(hashtable_get(h, key_to_keep) != NULL);

    // test clear
    hashtable_clear(h);
    assert(hashtable_is_empty(h));
    assert(hashtable_length(h) == 0);

    // test iterator
    hashtable_clear(h);
    hashtable_set(h, key1, payload1);
    hashtable_set(h, key2, payload2);
    hashtable_set(h, key3, payload3);

    iterator *it = hashtable_create_iterator(h, mp);
    str *collated = new_str(mp, "");
    for_iterator(char, ptr, it) {
        str_cats(collated, ptr);
        str_cats(collated, ",");
    }
    // this is flaky, order depends on hashing function.
    assert(str_cmps(collated, "payload 1,payload 2,payload 3,") == 0);

    // a statistical test...
    int test_size = 1024;
    hashtable *bighash = new_hashtable(mp, test_size);
    for (int i = 0; i < test_size; i++) {
        str *s = new_str_random(mp, 6, 10);
        hashtable_set(bighash, s, s);
    }
    // now we should find out distribution.
    int used_slots = 0;
    int free_slots = 0;
    int chain_len = 0;
    hashtable_node *n = NULL;
    int longest_chain_len = 0;
    for (int i = 0; i < bighash->capacity; i++) {
        if (bighash->items_arr[i] == NULL)
            free_slots++;
        else {
            used_slots++;
            if (bighash->items_arr[i]->next != NULL) {
                n = bighash->items_arr[i];
                chain_len = 1;
                while (n != NULL) {
                    chain_len++;
                    n = n->next;
                }
                if (chain_len > longest_chain_len)
                    longest_chain_len = chain_len;

            }
        }
    }
    // make sure distribution is more than 75% (it's usually 63% used - 37% free)
    // longest chain was 6 in a 1024 items test
    assert(used_slots > test_size / 2);
    assert(longest_chain_len < test_size / 100);
    

    // mempool_print_allocations(mp, stdout);
    mempool_release(mp);
}
#endif


// --------------------------------------------------------

heap *new_heap(mempool *mp, bool max, comparator_function *cmp);
void  heap_add(heap *h, void *item);  // O(lg N)
void  heap_delete(heap *h, void *item); // O(1)
void *heap_peek(heap *h);    // O(1)
void *heap_extract(heap *h); // O(1)

void heap_unit_tests() {
    // something something.
}

// --------------------------------------------------------

typedef struct graph graph;
typedef struct graph_node graph_node;
graph *new_graph(mempool *mp);
int    graph_length(graph *g);
bool   graph_empty(graph *g);
bool   graph_add_node(graph *g, str *key, void *node_data);
bool   graph_add_link(graph *g, str *key_from, str *key_to, void *vertex_data);
bool   graph_remove_node(graph *g, str *key_from, str *key_to, void *vertex_data);
bool   graph_remove_link(graph *g, str *key_from, str *key_to, void *vertex_data);
void  *graph_get_node(graph *g, str *key);
llist *graph_get_links_from(graph *g, str *node_key);
llist *graph_get_links_to(graph *g, str *node_key);
bool   graph_acyclic(graph *g);
int    graph_number_of_trees(graph *g);
llist *graph_topological_sort(graph *g);
llist *graph_shortest_path(graph *g, str *from_key, str *to_key);
iterator *graph_create_bfs_iterator(graph *g, mempool *m, str *first_key);
iterator *graph_create_dfs_iterator(graph *g, mempool *m, str *first_key);

void graph_unit_tests() {
    // oh boy, this is going to be fun!
}


// --------------------------------------------------------

// load everything in a list, create iterator of the list,
// filter it,
// map it,
// sort it,
// group it,
// then: take one value, or reduce() to a value, or collect to a list

long iterator_count(iterator *it) {
    long count = 0;
    for (it->reset(it); it->valid(it); it->next(it))
        count++;
    return count;
}

iterator *iterator_filter(iterator *it, filterer_function filter);
iterator *iterator_map(iterator *it, mapper_function filter);
void *iterator_reduce(iterator *it, reducer_function filter, void *initial_value);

hashtable *iterator_group(iterator *it, classifier_function classifier); // hashtable of llists per group

void *iterator_first(iterator *it) {
    void *item = it->reset(it);
    if (it->valid(it))
        return item;
    return NULL;
}

void *iterator_last(iterator *it) {
    void *valid_item = NULL;
    void *item = it->reset(it);
    while (it->valid(it)) {
        valid_item = item;
        item = it->next(it);
    }
    return valid_item;
}

llist *iterator_collect(iterator *it, mempool *mp) {
    llist *l = new_llist(mp);
    for_iterator(void, ptr, it)
        llist_add(l, ptr);
    return l;
}

#ifdef INCLUDE_UNIT_TESTS
void iterator_unit_tests() {
    // test the following
    // long iterator_count(iterator *it);
    // iterator *iterator_filter(iterator *it, filterer_function filter);
    // iterator *iterator_map(iterator *it, mapper_function filter);
    // hashtable *iterator_group(iterator *it, classifier_function classifier); // hashtable of llists per group
    // void *iterator_reduce(iterator *it, reducer_function filter, void *initial_value);
    // void *iterator_first(iterator *it);
    // void *iterator_last(iterator *it);
    // llist *iterator_collect(iterator *it, mempool *mp);
}
#endif

// ---------------------------------------------------

#ifdef INCLUDE_UNIT_TESTS
void all_data_structs_unit_tests() {
    queue_unit_tests();
    stack_unit_tests();
    llist_unit_tests();
    bstree_unit_tests();
    hashtable_unit_tests();
    heap_unit_tests();
    graph_unit_tests();
    iterator_unit_tests();
}
#endif

