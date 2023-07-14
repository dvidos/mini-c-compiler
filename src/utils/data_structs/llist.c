#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "llist.h"


typedef struct llist_node {
    void *data;
    struct llist_node *next;
    struct llist_node *prev;
} llist_node;

struct llist {
    int items_count;
    struct llist_node *head;
    struct llist_node *tail;
    iterator *base_iterator;
    mempool *mempool;
};

llist *new_llist(mempool *mp) {
    llist *l = mpalloc(mp, llist);
    memset(l, 0, sizeof(llist));
    l->mempool = mp;
    l->base_iterator = llist_create_iterator(l, mp);
    return l;
}

llist *new_llist_of(mempool *mp, int num, ...) {
    llist *l = new_llist(mp);
    if (num > 0) {
        va_list arguments;
        va_start(arguments, num);
        while (num-- > 0)
            llist_add(l, va_arg(arguments, void *));
        va_end(arguments);
    }
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
    llist_node *n = mpalloc(l->mempool, llist_node);
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

void llist_add_all(llist *l, llist *source) {
    llist_node *n = source->head;
    while (n != NULL) {
        llist_add(l, n->data);
        n = n->next;
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
        llist_node *new_node = mpalloc(l->mempool, llist_node);
        memset(new_node, 0, sizeof(llist_node));
        new_node->data = item;
        new_node->next = l->head;
        l->head = new_node;
        if (new_node->next != NULL)
            new_node->next->prev = new_node;
        l->items_count++;

    } else {
        // index in the middle
        llist_node *new_node = mpalloc(l->mempool, llist_node);
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
    llist *new_list = mpalloc(l->mempool, llist);
    memset(new_list, 0, sizeof(llist));
    new_list->mempool = l->mempool;
    llist_node *n = l->tail;
    while (n != NULL) {
        llist_add(new_list, n->data);
        n = n->prev;
    }
    return new_list;
}

static comparator_func *user_sort_comparator;
static int llist_sort_comparator(const void *a, const void *b) {
    // qsort passes pointers to the data to be compared.
    // but out data are already pointers to the user data.
    return user_sort_comparator(*(const void **)a, *(const void **)b);
}

int llist_find_first(llist *l, comparator_func *compare, void *item) {
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

int llist_find_last(llist *l, comparator_func *compare, void *item) {
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

llist *llist_sort(llist *l, comparator_func *compare, mempool *mp) {
    // allocate an array with all the pointers, then sort them, then create a new list

    mempool *scratch = new_mempool();
    void **ptr_array = mpallocn(scratch, l->items_count * sizeof(void *), "llist_ptr_array");
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

llist *llist_unique(llist *l, comparator_func *compare, mempool *mp) {
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

llist *llist_filter(llist *l, filterer_func *filter, mempool *mp) {
    llist *new_list = mpalloc(mp, llist);
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

llist *llist_map(llist *l, mapper_func *map, mempool *mp) {
    llist *new_list = mpalloc(l->mempool, llist);
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

void *llist_reduce(llist *l, reducer_func *reduce, void *initial_value, mempool *mp) {
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
    it_state->curr = (it_state->curr == NULL) ? NULL : it_state->curr->next;
    return it_state->curr == NULL ? NULL : it_state->curr->data;
}

static void *llist_iterator_lookahead(iterator *it, int times) {
    llist_iterator_private_state *it_state = (llist_iterator_private_state *)it->private_data;
    llist_node *ahead = it_state->curr;

    // if went past the list end, return NULL
    while (times-- > 0 && ahead != NULL)
        ahead = ahead->next;

    return ahead->data;
}

void *llist_iteration_reset(llist *l) {
    return llist_iterator_reset(l->base_iterator);
}

bool  llist_iteration_valid(llist *l) {
    return llist_iterator_valid(l->base_iterator);
}

void *llist_iteration_next(llist *l) {
    return llist_iterator_next(l->base_iterator);
}

iterator *llist_create_iterator(llist *l, mempool *mp) {
    llist_iterator_private_state *it_state = mpalloc(mp, llist_iterator_private_state);
    memset(it_state, 0, sizeof(llist_iterator_private_state));
    it_state->list = l;

    iterator *it = mpalloc(mp, iterator);
    it->private_data = it_state;
    it->reset = llist_iterator_reset;
    it->valid = llist_iterator_valid;
    it->next = llist_iterator_next;
    it->lookahead = llist_iterator_lookahead;
    return it;
}

iterator *llist_base_iterator(llist *l) {
    return l->base_iterator;
}

hashtable *llist_group(llist *l, classifier_func *classify, mempool *mp) {
    // we can derive groups, then create a hashtable with lists of the contents of each group.
    return NULL;
}

#ifdef INCLUDE_UNIT_TESTS
static bool __llist_unit_test_filter(void *item) {
    return strcmp((char *)item, "A") == 0;
}
static void *__llist_unit_test_map(void *item, mempool *mp) {
    char *src = (char *)item;
    char *dst = mpallocn(mp, strlen(src) * 2 + 4, "string mapping");
    strcpy(dst, src);
    strcat(dst, "-");
    strcat(dst, src);
    return dst;
}
static void *__llist_unit_test_reduce(void *item, void *prev_value, mempool *mp) {
    char *reduced = mpallocn(mp, strlen((char *)prev_value) + 1 + strlen((char *)item) + 1, "reduced value");

    strcpy(reduced, (char *)prev_value);
    if (strlen(reduced) > 0)
        strcat(reduced, ",");
    strcat(reduced, (char *)item);

    return reduced;
}

void llist_unit_tests() {
    mempool *mp = new_mempool();
    char *s;

    void *a = mpallocn(mp, 4, "dummy data");
    void *b = mpallocn(mp, 4, "dummy data");
    void *c = mpallocn(mp, 4, "dummy data");
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
    assert(llist_find_first(l, (comparator_func *)strcmp, a) == 0);
    assert(llist_find_first(l, (comparator_func *)strcmp, c) == -1);
    assert(llist_find_last(l, (comparator_func *)strcmp, a) == 2);
    assert(llist_find_last(l, (comparator_func *)strcmp, c) == -1);

    // sort
    llist_clear(l);
    llist_add(l, b);
    llist_add(l, c);
    llist_add(l, a);
    llist *sorted = llist_sort(l, (comparator_func *)strcmp, mp);
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
    llist *uniq = llist_unique(l, (comparator_func *)strcmp, mp);
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

