#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "list.h"


typedef struct list_node {
    void *data;
    struct list_node *next;
    struct list_node *prev;
} list_node;

struct list {
    int items_count;
    struct list_node *head;
    struct list_node *tail;
    iterator *base_iterator;
    mempool *mempool;
};

list *new_list(mempool *mp) {
    list *l = mpalloc(mp, list);
    memset(l, 0, sizeof(list));
    l->mempool = mp;
    l->base_iterator = list_create_iterator(l, mp);
    return l;
}

list *new_list_of(mempool *mp, int num, ...) {
    list *l = new_list(mp);
    if (num > 0) {
        va_list arguments;
        va_start(arguments, num);
        while (num-- > 0)
            list_add(l, va_arg(arguments, void *));
        va_end(arguments);
    }
    return l;
}

int list_length(list *l) {
    return l->items_count;
}

bool list_is_empty(list *l) {
    return l->items_count == 0;
}

void *list_get(list *l, int index) {
    list_node *n = l->head;
    while (index-- > 0 && n != NULL)
        n = n->next;
    
    return n == NULL ? NULL : n->data;
}

void list_add(list *l, void *item) {
    list_node *n = mpalloc(l->mempool, list_node);
    memset(n, 0, sizeof(list_node));
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

void list_add_all(list *l, list *source) {
    list_node *n = source->head;
    while (n != NULL) {
        list_add(l, n->data);
        n = n->next;
    }
}

bool list_contains(list *l, void *item) {
    list_node *n = l->head;
    while (n != NULL) {
        if (n->data == item)
            return true;
        n = n->next;
    }
    return false;
}

int list_index_of(list *l, void *item) {
    list_node *n = l->head;
    int index = 0;
    while (n != NULL) {
        if (n->data == item)
            return index;
        n = n->next;
        index++;
    }
    return -1;
}

bool list_insert_at(list *l, int index, void *item) {
    if (index < 0 || index > l->items_count) {
        // invalid location
        return false;

    } else if (index == l->items_count) {
        list_add(l, item);
        return true;

    } else if (index == 0) {
        // index at the start
        list_node *new_node = mpalloc(l->mempool, list_node);
        memset(new_node, 0, sizeof(list_node));
        new_node->data = item;
        new_node->next = l->head;
        l->head = new_node;
        if (new_node->next != NULL)
            new_node->next->prev = new_node;
        l->items_count++;

    } else {
        // index in the middle
        list_node *new_node = mpalloc(l->mempool, list_node);
        memset(new_node, 0, sizeof(list_node));
        new_node->data = item;
        new_node->next = l->head;
        list_node *n = l->head;
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

bool list_remove_at(list *l, int index) {
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
        list_node *n = l->head;
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

void list_clear(list *l) {
    l->head = NULL;
    l->tail = NULL;
    l->items_count = 0;
}

list *list_reverse(list *l) {
    list *nl = mpalloc(l->mempool, list);
    memset(nl, 0, sizeof(list));
    nl->mempool = l->mempool;
    list_node *n = l->tail;
    while (n != NULL) {
        list_add(nl, n->data);
        n = n->prev;
    }
    return nl;
}

static comparator_func *user_sort_comparator;
static int list_sort_comparator(const void *a, const void *b) {
    // qsort passes pointers to the data to be compared.
    // but out data are already pointers to the user data.
    return user_sort_comparator(*(const void **)a, *(const void **)b);
}

int list_find_first(list *l, comparator_func *compare, void *item) {
    list_node *n = l->head;
    int index = 0;
    while (n != NULL) {
        if (compare(n->data, item) == 0)
            return index;
        n = n->next;
        index++;
    }
    return -1;
}

int list_find_last(list *l, comparator_func *compare, void *item) {
    list_node *n = l->tail;
    int index = l->items_count - 1;
    while (n != NULL) {
        if (compare(n->data, item) == 0)
            return index;
        n = n->prev;
        index--;
    }
    return -1;
}

list *list_sort(list *l, comparator_func *compare, mempool *mp) {
    // allocate an array with all the pointers, then sort them, then create a new list

    mempool *scratch = new_mempool();
    void **ptr_array = mpallocn(scratch, l->items_count * sizeof(void *), "list_ptr_array");
    list_node *n = l->head;
    int i = 0;
    while (n != NULL) {
        ptr_array[i++] = n->data;
        n = n->next;
    }

    // qsort() will pass a pointer to the pointer of the actual data
    // so, we need an internal function, and to know which comparer to call.
    user_sort_comparator = compare;
    qsort(ptr_array, l->items_count, sizeof(void *), list_sort_comparator);

    // now, create a new list with the sorted data
    list *nl = new_list(mp);
    for (int i = 0; i < l->items_count; i++)
        list_add(nl, ptr_array[i]);

    // we can free the scratch pool
    mempool_release(scratch);
    return nl;
}

list *list_unique(list *l, comparator_func *compare, mempool *mp) {
    // simple approach for now, O(n^2 / 2)
    list *nl = new_list(mp);
    list_node *n = l->head;
    while (n != NULL) {
        
        if (list_find_first(nl, compare, n->data) == -1)
            list_add(nl, n->data);
        n = n->next;
    }

    return nl;
}

list *list_filter(list *l, filterer_func *filter, mempool *mp) {
    list *nl = mpalloc(mp, list);
    memset(nl, 0, sizeof(list));
    nl->mempool = mp;

    list_node *n = l->head;
    while (n != NULL) {
        if (filter(n->data))
            list_add(nl, n->data);
        n = n->next;
    }
    return nl;
}

list *list_map(list *l, mapper_func *map, mempool *mp) {
    list *nl = mpalloc(l->mempool, list);
    memset(nl, 0, sizeof(list));
    nl->mempool = l->mempool;

    list_node *n = l->head;
    while (n != NULL) {
        void *new_data = map(n->data, l->mempool);
        list_add(nl, new_data);
        n = n->next;
    }
    return nl;
}

void *list_reduce(list *l, reducer_func *reduce, void *initial_value, mempool *mp) {
    void *p = initial_value;
    list_node *n = l->head;
    while (n != NULL) {
        p = reduce(n->data, p, mp);
        n = n->next;
    }
    return p;
}

typedef struct list_iterator_private_state {
    list *list;
    list_node *curr;
} list_iterator_private_state;

static void *list_iterator_reset(iterator *it) {
    list_iterator_private_state *it_state = (list_iterator_private_state *)it->private_data;
    it_state->curr = it_state->list->head;
    return it_state->curr == NULL ? NULL : it_state->curr->data;
}

static bool list_iterator_valid(iterator *it) {
    list_iterator_private_state *it_state = (list_iterator_private_state *)it->private_data;
    return it_state->curr != NULL;
}

static void *list_iterator_next(iterator *it) {
    list_iterator_private_state *it_state = (list_iterator_private_state *)it->private_data;
    it_state->curr = (it_state->curr == NULL) ? NULL : it_state->curr->next;
    return it_state->curr == NULL ? NULL : it_state->curr->data;
}

static void *list_iterator_lookahead(iterator *it, int times) {
    list_iterator_private_state *it_state = (list_iterator_private_state *)it->private_data;
    list_node *ahead = it_state->curr;

    // if went past the list end, return NULL
    while (times-- > 0 && ahead != NULL)
        ahead = ahead->next;

    return ahead->data;
}

void *list_iteration_reset(list *l) {
    return list_iterator_reset(l->base_iterator);
}

bool  list_iteration_valid(list *l) {
    return list_iterator_valid(l->base_iterator);
}

void *list_iteration_next(list *l) {
    return list_iterator_next(l->base_iterator);
}

iterator *list_create_iterator(list *l, mempool *mp) {
    list_iterator_private_state *it_state = mpalloc(mp, list_iterator_private_state);
    memset(it_state, 0, sizeof(list_iterator_private_state));
    it_state->list = l;

    iterator *it = mpalloc(mp, iterator);
    it->private_data = it_state;
    it->reset = list_iterator_reset;
    it->valid = list_iterator_valid;
    it->next = list_iterator_next;
    it->lookahead = list_iterator_lookahead;
    return it;
}

iterator *list_base_iterator(list *l) {
    return l->base_iterator;
}

hashtable *list_group(list *l, classifier_func *classify, mempool *mp) {
    // we can derive groups, then create a hashtable with lists of the contents of each group.
    return NULL;
}

#ifdef INCLUDE_UNIT_TESTS
static bool __list_unit_test_filter(void *item) {
    return strcmp((char *)item, "A") == 0;
}
static void *__list_unit_test_map(void *item, mempool *mp) {
    char *src = (char *)item;
    char *dst = mpallocn(mp, strlen(src) * 2 + 4, "string mapping");
    strcpy(dst, src);
    strcat(dst, "-");
    strcat(dst, src);
    return dst;
}
static void *__list_unit_test_reduce(void *item, void *prev_value, mempool *mp) {
    char *reduced = mpallocn(mp, strlen((char *)prev_value) + 1 + strlen((char *)item) + 1, "reduced value");

    strcpy(reduced, (char *)prev_value);
    if (strlen(reduced) > 0)
        strcat(reduced, ",");
    strcat(reduced, (char *)item);

    return reduced;
}

void list_unit_tests() {
    mempool *mp = new_mempool();
    char *s;

    void *a = mpallocn(mp, 4, "dummy data");
    void *b = mpallocn(mp, 4, "dummy data");
    void *c = mpallocn(mp, 4, "dummy data");
    strcpy(a, "A");
    strcpy(b, "B");
    strcpy(c, "C");

    // test initial conditions
    list *l = new_list(mp);
    assert(l != NULL);
    assert(l->items_count == 0);
    assert(l->head == NULL);
    assert(l->tail == NULL);
    assert(list_length(l) == 0);
    assert(list_is_empty(l));
    assert(list_get(l, 0) == NULL);
    assert(!list_contains(l, a));

    // adding, getting
    list_add(l, a);
    list_add(l, b);
    list_add(l, c);
    assert(l->items_count == 3);
    assert(l->head != NULL && l->head->data == a);
    assert(l->tail != NULL && l->tail->data == c);
    assert(list_get(l, 0) == a);
    assert(list_get(l, 1) == b);
    assert(list_get(l, 2) == c);

    // test clearing
    list_clear(l);
    assert(l->items_count == 0);
    assert(l->head == NULL);
    assert(l->tail == NULL);

    // test contains
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_add(l, c);
    assert(list_contains(l, b));
    list_clear(l);
    assert(!list_contains(l, b));

    // insert start
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_insert_at(l, 0, c);
    assert(list_get(l, 0) == c);
    assert(list_get(l, 1) == a);
    assert(list_get(l, 2) == b);

    // insert middle
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_insert_at(l, 1, c);
    assert(list_get(l, 0) == a);
    assert(list_get(l, 1) == c);
    assert(list_get(l, 2) == b);

    // insert at end
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_insert_at(l, 2, c);
    assert(list_get(l, 0) == a);
    assert(list_get(l, 1) == b);
    assert(list_get(l, 2) == c);

    // delete from start
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_add(l, c);
    list_remove_at(l, 0);
    assert(list_get(l, 0) == b);
    assert(list_get(l, 1) == c);

    // delete from middle
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_add(l, c);
    list_remove_at(l, 1);
    assert(list_get(l, 0) == a);
    assert(list_get(l, 1) == c);

    // delete from end
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_add(l, c);
    list_remove_at(l, 2);
    assert(list_get(l, 0) == a);
    assert(list_get(l, 1) == b);

    // delete the last one
    list_clear(l);
    list_add(l, a);
    list_remove_at(l, 0);
    assert(list_length(l) == 0);
    assert(l->head == NULL);
    assert(l->tail == NULL);

    // reverse
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_add(l, c);
    list *reversed = list_reverse(l);
    assert(list_get(reversed, 0) == c);
    assert(list_get(reversed, 1) == b);
    assert(list_get(reversed, 2) == a);

    // find first + find last
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_add(l, a);
    assert(list_find_first(l, (comparator_func *)strcmp, a) == 0);
    assert(list_find_first(l, (comparator_func *)strcmp, c) == -1);
    assert(list_find_last(l, (comparator_func *)strcmp, a) == 2);
    assert(list_find_last(l, (comparator_func *)strcmp, c) == -1);

    // sort
    list_clear(l);
    list_add(l, b);
    list_add(l, c);
    list_add(l, a);
    list *sorted = list_sort(l, (comparator_func *)strcmp, mp);
    assert(list_get(sorted, 0) == a);
    assert(list_get(sorted, 1) == b);
    assert(list_get(sorted, 2) == c);

    // unique
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_add(l, b);
    list_add(l, a);
    list_add(l, c);
    list_add(l, a);
    assert(list_length(l) == 6);
    list *uniq = list_unique(l, (comparator_func *)strcmp, mp);
    assert(list_length(uniq) == 3);
    assert(list_get(uniq, 0) == a);
    assert(list_get(uniq, 1) == b);
    assert(list_get(uniq, 2) == c);

    // filter
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_add(l, b);
    list_add(l, a);
    list_add(l, c);
    list_add(l, a);
    assert(list_length(l) == 6);
    list *filtered = list_filter(l, __list_unit_test_filter, mp);
    assert(list_length(filtered) == 3);
    assert(list_get(filtered, 0) == a);
    assert(list_get(filtered, 1) == a);
    assert(list_get(filtered, 2) == a);

    // map
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_add(l, c);
    list *mapped = list_map(l, __list_unit_test_map, mp);
    assert(list_length(mapped) == 3);
    assert(strcmp(list_get(mapped, 0), "A-A") == 0);
    assert(strcmp(list_get(mapped, 1), "B-B") == 0);
    assert(strcmp(list_get(mapped, 2), "C-C") == 0);

    // reduce
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_add(l, c);
    void *result = list_reduce(l, __list_unit_test_reduce, "", mp);
    assert(result != NULL);
    assert(strcmp(result, "A,B,C") == 0);

    // create_iterator
    list_clear(l);
    list_add(l, a);
    list_add(l, b);
    list_add(l, c);
    iterator *it = list_create_iterator(l, mp);
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

