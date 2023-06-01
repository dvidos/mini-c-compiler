#include <stdlib.h>
#include <string.h>
#include "unit_tests.h"
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
// static iterator *_create_iterator(list *l);
static void _for_each(list *l, visitor_func *visitor, void *extra_data);
static int _find(list *l, void *criteria, matcher_func *matcher, void *extra_data);
static void _append_all(list *l, list *other);
static void _sort(list *l, comparer_func *comparer, void *extra_data);
static int _bin_search(list *l, void *criteria, comparer_func *comparer, void *extra_data);
static list *_filter(list *l, filter_func *filter, void *extra_data);
static list *_map(list *l, mapper_func *mapper, void *extra_data);
static void *_reduce(list *l, void *init_value, reducer_func *reduce, void *extra_data);
static list *_deduplicate(list *l, matcher_func *matcher, void *extra_data);
static string *_join(list *l, string *separator, to_string_func *to_str);
static unsigned long _hash(list *l, hasher_func *hasher);
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
    // .create_iterator = _create_iterator,
    // .for_each = _for_each,
    // .find = _find,
    // .append_all = _append_all,
    // .sort = _sort,
    // .bin_search = _bin_search,
    // .filter = _filter,
    // .map = _map,
    // .reduce = _reduce,
    // .deduplicate = _deduplicate,
    // .join = _join,
    // .hash = _hash,
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
    if (index < 0 || index > data->length)
        return;

    if (index == data->length) {
        _add(l, item);
        return;
    }

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




// struct list_iterator_priv_data {
//     list *l;
//     int index;
// };
// static void *_list_iterator_reset(iterator *iter) {
//     struct list_iterator_priv_data *data = (struct list_iterator_priv_data *)iter->priv_data;
//     data->index = 0;
//     return _get(data->l, data->index);
// }
// static bool _list_iterator_valid(iterator *iter) {
//     struct list_iterator_priv_data *data = (struct list_iterator_priv_data *)iter->priv_data;
//     return data->index >= 0 && data->index < data->l->v->length(data->l);
// }
// static void *_list_iterator_next(iterator *iter) {
//     struct list_iterator_priv_data *data = (struct list_iterator_priv_data *)iter->priv_data;
//     data->index += 1;
//     return _get(data->l, data->index);
// }
// static void _list_iterator_free(iterator *iter) {
//     free(iter);  // we do not free the list here
// }
// struct iterator_vtable list_iterator_vtable = {
//     .reset = _list_iterator_reset,
//     .valid = _list_iterator_valid,
//     .next = _list_iterator_next,
//     .free = _list_iterator_free
// };

// static iterator *_create_iterator(list *l) {
//     struct list_iterator_priv_data *iter_data = malloc(sizeof(struct list_iterator_priv_data));
//     iter_data->l = l;
//     iter_data->index = 0;

//     iterator *it = malloc(sizeof(iterator));
//     it->priv_data = iter_data;
//     it->v = &list_iterator_vtable;

//     return it;
// }

static void _for_each(list *l, visitor_func *visitor, void *extra_data) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (visitor == NULL)
        return;
    for (int i = 0; i < data->length; i++) {
        visitor(data->buffer[i], extra_data);
    }
}

static int _find(list *l, void *criteria, matcher_func *matcher, void *extra_data) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (matcher == NULL)
        return -1;
    
    for (int i = 0; i < data->length; i++) {
        if (matcher(data->buffer[i], criteria, extra_data))
            return i;
    }
    return -1;
}

static void _append_all(list *l, list *other) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    struct list_priv_data *other_data = (struct list_priv_data *)other->priv_data;

    _ensure_capacity(data, data->length + other_data->length);
    for (int i = 0; i < other_data->length; i++) {
        _add(l, other_data->buffer[i]);
    }
}



static comparer_func *_client_comparer_func = NULL;
static void *_client_compare_extra_data = NULL;
static int _list_comparer_fn(const void *a, const void *b) { // not thread safe
    if (_client_comparer_func == NULL)
        return 0;
    
    return _client_comparer_func(a, b, _client_compare_extra_data);
}

static void _sort(list *l, comparer_func *comparer, void *extra_data) {
    if (comparer == NULL)
        return;

    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    _client_comparer_func = comparer;
    _client_compare_extra_data = extra_data;
    qsort(data->buffer, data->length, sizeof(void *), _list_comparer_fn); // sorting in place.
    _client_comparer_func = NULL;
    _client_compare_extra_data = NULL;
}

static int _bin_search(list *l, void *criteria, comparer_func *comparer, void *extra_data) {
    if (comparer == NULL)
        return -1;
    
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    _client_comparer_func = comparer;
    _client_compare_extra_data = extra_data;
    bsearch(criteria, data->buffer, data->length, sizeof(void *), _list_comparer_fn);
    _client_comparer_func = NULL;
    _client_compare_extra_data = NULL;
}

static list *_filter(list *l, filter_func *filter, void *extra_data) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    if (filter == NULL)
        return NULL;
    
    list *result = new_list();
    for (int i = 0; i < data->length; i++) {
        if (!filter(data->buffer[i], extra_data))
            continue;
        
        _add(result, data->buffer[i]);
    }
    return result;
}

static list *_map(list *l, mapper_func *mapper, void *extra_data) {
    if (mapper == NULL)
        return NULL;
    
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    list *mapped_list = new_list();
    for (int i = 0; i < data->length; i++) {
        void *mapped_item = mapper(data->buffer[i], extra_data);
        _add(mapped_list, mapped_item);
    }

    return mapped_list;
}

static void *_reduce(list *l, void *init_value, reducer_func *reduce, void *extra_data) {
    if (reduce == NULL)
        return NULL;
    
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    list *mapped_list = new_list();
    void *running_value = init_value;
    for (int i = 0; i < data->length; i++) {
        running_value = reduce(data->buffer[i], running_value, extra_data);
    }

    return running_value;
}

static list *_deduplicate(list *l, matcher_func *matcher, void *extra_data) {
    if (matcher == NULL)
        return NULL;

    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    list *deduped = new_list();
    for (int i = 0; i < data->length; i++) {
        void *item = data->buffer[i];
        if (_find(deduped, item, matcher, extra_data))
            continue;
        _add(deduped, item); // we could clone the item here
    }

    return deduped;
}

static string *_join(list *l, string *separator, to_string_func *to_str) {
    if (to_str == NULL)
        return NULL;
    
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;
    string *joined = new_string();
    for (int i = 0; i < data->length; i++) {
        if (i > 0)
            joined->v->add(joined, separator);
        
        char *repr = to_str(data->buffer[i]);
        joined->v->adds(joined, repr);
        free(repr);
    }

    return joined;
}

static unsigned long _hash(list *l, hasher_func *hasher) {
    struct list_priv_data *data = (struct list_priv_data *)l->priv_data;

    unsigned long hash = 0;

    if (hasher == NULL) {
        for (int i = 0; i < data->length; i++) {
            hash = (hash << 5) ^ (long)data->buffer[i];
        }
    } else {
        for (int i = 0; i < data->length; i++) {
            hash = (hash << 5) ^ hasher(data->buffer[i]);
        }
    }

    return hash;
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

#ifdef INCLUDE_UNIT_TESTS

static char *_strz_to_strz(void *p) {
    return strdup((char *)p);
}

static unsigned long _strz_hasher(void *p) {
    long l = 0;
    char *s = p;
    while (*s)
        l = (l << 7) ^ *s++;
    return l;
}

static bool _match_str_case_sensitive(const void *item, const void *criteria, void *extra_data) {
    return strcmp(item, criteria) == 0;
}

static bool _match_str_case_insensitive(const void *item, const void *criteria, void *extra_data) {
    return strcasecmp(item, criteria) == 0;
}

void list_unit_tests() {

    char *s1 = "hello";
    char *s2 = "there";
    char *s3 = "world";
    char *r, *r1, *r2;
    int i;

    list *l = new_list();
    assert(l->v->length(l) == 0);
    l->v->free(l, NULL);

    // test clear works
    l = new_list();
    l->v->add(l, s1);
    assert(l->v->length(l) > 0);
    l->v->clear(l);
    assert(l->v->length(l) == 0);
    l->v->free(l, NULL);

    // test add, length, get, set
    l = new_list();
    l->v->add(l, s1);
    l->v->add(l, s2);
    assert(l->v->length(l) == 2);
    assert(l->v->get(l, -1) == NULL);
    assert(l->v->get(l, 0) == s1);
    assert(l->v->get(l, 1) == s2);
    assert(l->v->get(l, 2) == NULL);
    // set second item, and check
    l->v->set(l, 1, s3);
    assert(l->v->get(l, 1) == s3);
    // setting at end of list should succeed
    assert(l->v->length(l) == 2);
    assert(l->v->get(l, 2) == NULL);
    l->v->set(l, 2, s3);
    assert(l->v->length(l) == 3);
    assert(l->v->get(l, 2) == s3);
    // setting beyond end of list should fail
    assert(l->v->length(l) == 3);
    assert(l->v->get(l, 5) == NULL);
    l->v->set(l, 5, s3);
    assert(l->v->length(l) == 3);
    assert(l->v->get(l, 5) == NULL);
    l->v->free(l, NULL);

    // index_of
    l = new_list();
    l->v->add(l, s1);
    l->v->add(l, s2);
    assert(l->v->index_of(l, s1) == 0);
    assert(l->v->index_of(l, s2) == 1);
    assert(l->v->index_of(l, s3) == -1);
    l->v->add(l, s3);
    assert(l->v->index_of(l, s3) == 2);
    l->v->free(l, NULL);

    // insert_at (head, mid, tail)
    l = new_list();
    l->v->add(l, s1);
    l->v->add(l, s2);
    l->v->insert_at(l, 0, s3);
    assert(l->v->index_of(l, s3) == 0);
    assert(l->v->index_of(l, s1) == 1);
    assert(l->v->index_of(l, s2) == 2);
    l->v->clear(l);
    l->v->add(l, s1);
    l->v->add(l, s2);
    l->v->insert_at(l, 2, s3);
    assert(l->v->index_of(l, s1) == 0);
    assert(l->v->index_of(l, s2) == 1);
    assert(l->v->index_of(l, s3) == 2);
    l->v->clear(l);
    l->v->add(l, s1);
    l->v->add(l, s2);
    l->v->insert_at(l, 1, s3);
    assert(l->v->index_of(l, s1) == 0);
    assert(l->v->index_of(l, s2) == 2);
    assert(l->v->index_of(l, s3) == 1);
    l->v->free(l, NULL);

    // remove_at
    l = new_list();
    l->v->add(l, s1);
    l->v->add(l, s2);
    l->v->add(l, s3);
    assert(l->v->index_of(l, s1) == 0);
    assert(l->v->index_of(l, s2) == 1);
    assert(l->v->index_of(l, s3) == 2);
    l->v->remove_at(l, 0);
    assert(l->v->index_of(l, s1) == -1);
    assert(l->v->index_of(l, s2) == 0);
    assert(l->v->index_of(l, s3) == 1);
    l->v->clear(l);
    l->v->add(l, s1);
    l->v->add(l, s2);
    l->v->add(l, s3);
    assert(l->v->index_of(l, s1) == 0);
    assert(l->v->index_of(l, s2) == 1);
    assert(l->v->index_of(l, s3) == 2);
    l->v->remove_at(l, 2);
    assert(l->v->index_of(l, s1) == 0);
    assert(l->v->index_of(l, s2) == 1);
    assert(l->v->index_of(l, s3) == -1);
    l->v->clear(l);
    l->v->add(l, s1);
    l->v->add(l, s2);
    l->v->add(l, s3);
    assert(l->v->index_of(l, s1) == 0);
    assert(l->v->index_of(l, s2) == 1);
    assert(l->v->index_of(l, s3) == 2);
    l->v->remove_at(l, 1);
    assert(l->v->index_of(l, s1) == 0);
    assert(l->v->index_of(l, s2) == -1);
    assert(l->v->index_of(l, s3) == 1);
    l->v->free(l, NULL);


    // enqueue, dequeue
    l = new_list();
    l->v->enqueue(l, s1);
    l->v->enqueue(l, s2);
    assert(l->v->length(l) == 2);
    r1 = l->v->dequeue(l);
    r2 = l->v->dequeue(l);
    assert(r1 = s1);
    assert(r2 = s2);
    assert(l->v->length(l) == 0);
    assert(l->v->dequeue(l) == NULL);
    l->v->free(l, NULL);

    // push, pop, peek
    l = new_list();
    l->v->push(l, s1);
    l->v->push(l, s2);
    assert(l->v->length(l) == 2);
    assert(l->v->peek(l) == s2);
    assert(r1 = s1);
    r2 = l->v->pop(l); // note popping into r2 first
    assert(l->v->peek(l) == s1);
    r1 = l->v->pop(l);
    assert(r2 = s2);
    assert(l->v->length(l) == 0);
    assert(l->v->pop(l) == NULL);
    assert(l->v->peek(l) == NULL);
    l->v->free(l, NULL);

    // l = new_list();
    // l->v->add(l, s1);
    // l->v->add(l, s2);
    // l->v->add(l, s3);
    // iterator *it = l->v->create_iterator(l);
    // r = it->v->reset(it);
    // assert(it->v->valid(it));
    // assert(r == s1);
    // r = it->v->next(it);
    // assert(it->v->valid(it));
    // assert(r == s2);
    // r = it->v->next(it);
    // assert(it->v->valid(it));
    // assert(r == s3);
    // r = it->v->next(it);
    // assert(!it->v->valid(it));
    // assert(r == NULL);
    // it->v->free(it);
    // l->v->free(l, NULL);

    // for_each(list *l, visitor_func *visitor);
    // find(list *l, void *criteria, matcher_func *matcher);
    // append_all(list *l, list *other);
    // sort(list *l, comparer_func *comparer);
    // bin_search(list *l, void *criteria, comparer_func *comparer);
    // filter(list *l, filter_func *filter);
    // map(list *l, mapper_func *mapper);
    // reduce(list *l, void *init_value, reducer_func *reduce);
    // deduplicate(list *l, comparer_func *comparer);

    // find
    // l = new_list();
    // l->v->add(l, s1);
    // l->v->add(l, s2);
    // l->v->add(l, s3);
    // i = l->v->find(l, "WORLD", _match_str_case_sensitive, NULL);
    // assert(i == -1);
    // i = l->v->find(l, "WORLD", _match_str_case_insensitive, NULL);
    // assert(i == 2);
    // i = l->v->find(l, "NOT FOUND", _match_str_case_insensitive, NULL);
    // assert(i == -1);
    // l->v->free(l, NULL);

    // // join, hash
    // l = new_list();
    // l->v->add(l, s1);
    // l->v->add(l, s2);
    // l->v->add(l, s3);
    // string *separator = new_string_from("-");
    // string *joined = l->v->join(l, separator, _strz_to_strz);
    // assert(strcmp(joined->buffer, "hello-there-world") == 0);
    // joined->v->free(joined);
    // separator->v->free(separator);
    // long h = l->v->hash(l, _strz_hasher);
    // assert(h != 0); // actually 29533034653380 for ["hello", "there", "world"] in current implementation
    // l->v->free(l, NULL);
}

#endif // UNIT_TESTS
