#include <stdlib.h>
#include <string.h>
#include "../unit_tests.h"
#include "list.h"

struct list_priv_data {
    void **buffer;
    int length;    // in items
    int capacity;  // in items, multiply by sizeof(void *) to get bytes
};


static void _clear(list *l);
static int _length(list *l);
static void *_get(list *l, int index);
static void _add(list *l, void *item);
static void _set(list *l, int index, void *item);
static int _index_of(list *l, void *item);
static void _insert_at(list *l, int index, void *item);
static void _remove_at(list *l, int index);
static void *_dequeue(list *l); // remove from front
static void *_pop(list *l); // remove from end (last added)
static void *_peek(list *l);  // return last, without removing
static iterator *_create_iterator(list *l);
static void _for_each(list *l, visitor_func *visitor, void *extra_data);
static list *_find(list *l, void *criteria, matcher_func *matcher, void *extra_data);
static void _append_all(list *l, list *other);
static void _sort(list *l, comparer_func *comparer, void *extra_data);
static int _bin_search(list *l, void *criteria, comparer_func *comparer, void *extra_data);
static list *_filter(list *l, filter_func *filter, void *extra_data);
static list *_map(list *l, mapper_func *mapper, void *extra_data);
static void *_reduce(list *l, void *init_value, reducer_func *reduce, void *extra_data);
static void _deduplicate(list *l, comparer_func *comparer, void *extra_data);
static string *_join(list *l, string *separator, to_string_func *to_str);
static long _hash(list *l, hasher_func *hasher);
static void _free(list *l, visitor_func *free_item); 


static struct list_vtable vtable = {
    .clear = _clear,
    .length = _length,
    .get = _get,
    .add = _add,
    .set = _set,
    .index_of = _index_of,
    .insert_at = _insert_at,
    .remove_at = _remove_at,
    .enqueue = _add, // note same signature
    .dequeue = _dequeue,
    .push = _add, // note same signature
    .pop = _pop,
    .peek = _peek,
    .create_iterator = _create_iterator,
    .for_each = _for_each,
    .find = _find,
    .append_all = _append_all,
    .sort = _sort,
    .bin_search = _bin_search,
    .filter = _filter,
    .map = _map,
    .reduce = _reduce,
    .deduplicate = _deduplicate,
    .join = _join,
    .hash = _hash,
    .free = _free
};

list *new_list() {
    list *l = malloc(sizeof(list));

    struct list_priv_data *data = malloc(sizeof(struct list_priv_data));
    data->capacity = 10;
    data->buffer = malloc(data->capacity * sizeof(void *));
    data->length = 0;

    l->priv_data = data;
    l->v = &vtable;
    return l;
}

static void _clear(list *l) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    data->length = 0;
}

static int _length(list *l) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    return data->length;
}

static void *_get(list *l, int index) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (index < 0 || index >= data->length)
        return NULL;
    return data->buffer[index];
}

static inline void _ensure_capacity(struct list_priv_data *data, int capacity) {
    if (data->capacity < capacity) {
        while (data->capacity < capacity)
            data->capacity *= 2;
        data->buffer = realloc(data->buffer, data->capacity * sizeof(void *));
    }
}

static void _add(list *l, void *item) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    // ensure capacity
    _ensure_capacity(data, data->length + 1);
    data->buffer[data->length] = item;
    data->length += 1;
}

static void _set(list *l, int index, void *item) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (index < 0 || index >= data->length)
        return;
    
    data->buffer[index] = item;
}

static int _index_of(list *l, void *item) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;

    for (int i = 0; i < data->length; i++) {
        if (data->buffer[i] == item)
            return i;
    }

    return -1;
}

static void _insert_at(list *l, int index, void *item) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (index == data->length) {
        _add(l, item);
        return;
    }

    if (index < 0 || index >= data->length)
        return;
    
    _ensure_capacity(data, data->length + 1);
    memmove(&data->buffer[index + 1], &data->buffer[index], (data->length - index) * sizeof(void *));
    data->buffer[index] = item;
    data->length += 1;
}

static void _remove_at(list *l, int index) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (index < 0 || index >= data->length)
        return;
    
    memmove(&data->buffer[index], &data->buffer[index + 1], (data->length - index - 1) * sizeof(void *));
    data->length -= 1;
}

static void _enqueue(list *l, void *item) {
    _add(l, item);
}

static void *_dequeue(list *l) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (data->length == 0)
        return NULL;
    
    void *item = _get(l, 0);
    _remove_at(l, 0);
    return item;
}

static void _push(list *l, void *item) {
    _add(l, item);
}

static void *_pop(list *l) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (data->length == 0)
        return NULL;
    
    void *item = _get(l, data->length - 1);
    _remove_at(l, data->length - 1);
    return item;
}

static void *_peek(list *l) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (data->length == 0)
        return NULL;
    return _get(l, data->length - 1);
}




struct list_iterator_priv_data {
    list *l;
    int index;
};
static void *_list_iterator_reset(iterator *iter) {
    struct list_iterator_priv_data *data = (struct list_iterator_priv_data *)iter->priv_data;
    data->index = 0;
    return _get(data->l, data->index);
}
static bool _list_iterator_valid(iterator *iter) {
    struct list_iterator_priv_data *data = (struct list_iterator_priv_data *)iter->priv_data;
    return data->index >= 0 && data->index < data->l->v->length(data->l);
}
static void *_list_iterator_next(iterator *iter) {
    struct list_iterator_priv_data *data = (struct list_iterator_priv_data *)iter->priv_data;
    data->index += 1;
    return _get(data->l, data->index);
}
static void _list_iterator_free(iterator *iter) {
    free(iter);  // we do not free the list here
}
struct iterator_vtable list_iterator_vtable = {
    .reset = _list_iterator_reset,
    .valid = _list_iterator_valid,
    .next = _list_iterator_next,
    .free = _list_iterator_free
};

static iterator *_create_iterator(list *l) {
    struct list_iterator_priv_data *iter_data = malloc(sizeof(struct list_iterator_priv_data));
    iter_data->l = l;
    iter_data->index = 0;

    iterator *it = malloc(sizeof(iterator));
    it->priv_data = iter_data;
    it->v = &list_iterator_vtable;

    return it;
}

static void _for_each(list *l, visitor_func *visitor, void *extra_data) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (visitor == NULL)
        return;
    for (int i = 0; i < data->length; i++) {
        visitor(data->buffer[i], extra_data);
    }
}

static list *_find(list *l, void *criteria, matcher_func *matcher, void *extra_data) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (matcher == NULL)
        return NULL;
    for (int i = 0; i < data->length; i++) {
        if (matcher(data->buffer[i], criteria, extra_data))
            return data->buffer[i];
    }
    return NULL;
}

static void _append_all(list *l, list *other) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    struct list_priv_data *other_data = (struct list_priv_data *)other->priv_data;

    _ensure_capacity(data, data->length + other_data->length);
    for (int i = 0; i < other_data->length; i++) {
        _add(l, other_data->buffer[i]);
    }
}

static void _sort(list *l, comparer_func *comparer, void *extra_data) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    // need qsort()
}

static int _bin_search(list *l, void *criteria, comparer_func *comparer, void *extra_data) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    // need bsearch()
}

static list *_filter(list *l, filter_func *filter, void *extra_data) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (filter == NULL)
        return NULL;
    
    list *result = new_list();
    for (int i = 0; i < data->length; i++) {
        if (filter(data->buffer[i], extra_data))
            _add(result, data->buffer[i]);
    }
    return result;
}

static list *_map(list *l, mapper_func *mapper, void *extra_data) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    // needs work
}

static void *_reduce(list *l, void *init_value, reducer_func *reduce, void *extra_data) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    // needs work
}

static void _deduplicate(list *l, comparer_func *comparer, void *extra_data) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    // O(n^2) unfortunately
}

static string *_join(list *l, string *separator, to_string_func *to_str) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    // needs work as well.
}

static long _hash(list *l, hasher_func *hasher) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    // needs work as well
}

static void _free(list *l, visitor_func *free_item) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;

    if (free_item != NULL) {
        for (int i = 0; i < data->length; i++)
            free_item(data->buffer[i], NULL);
    }

    free(data->buffer);
    free(data);
    free(l);
}

// -------------------------------

#ifdef INCLUDE_UNIT_TESTS

void list_unit_tests() {

    char *s1 = "hello";
    char *s2 = "there";
    char *s3 = "world";

    list *l = new_list();
    assert(l->v->length(l) == 0);
    l->v->free(l, NULL);


    // test clear works
    l = new_list();
    l->v->add(l, s1);
    assert(l->v->length(l) > 0);
    l->v->clear(l);
    assert(l->v->length(l) == 0);

    // test add, length, get
    l->v->clear(l);
    l->v->add(l, s1);
    l->v->add(l, s2);
    assert(l->v->length(l) == 2);
    assert(l->v->get(l, -1) == NULL);
    assert(l->v->get(l, 0) == s1);
    assert(l->v->get(l, 1) == s2);
    assert(l->v->get(l, 2) == NULL);




    // clear(list *l);
    // length(list *l);
    // get(list *l, int index);
    // add(list *l, void *item);
    // set(list *l, int index, void *item);
    // index_of(list *l, void *item);
    // insert_at(list *l, int index, void *item);
    // remove_at(list *l, int index);
    // enqueue(list *l); // remove from front
    // dequeue(list *l); // remove from end (last added)
    // push(list *l, void *item); // same as add()
    // pop(list *l); // remove from end (last added)
    // peek(list *l);  // return last, without removing
    // create_iterator(list *l);
    // for_each(list *l, visitor_func *visitor);
    // find(list *l, void *criteria, matcher_func *matcher);
    // append_all(list *l, list *other);
    // sort(list *l, comparer_func *comparer);
    // bin_search(list *l, void *criteria, comparer_func *comparer);
    // filter(list *l, filter_func *filter);
    // map(list *l, mapper_func *mapper);
    // reduce(list *l, void *init_value, reducer_func *reduce);
    // deduplicate(list *l, comparer_func *comparer);
    // join(list *l, str *separator, to_str_func *to_str);
    // hash(list *l, hasher_func *hasher);

}

#endif // UNIT_TESTS
