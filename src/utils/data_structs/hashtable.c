#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "hashtable.h"


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
    hashtable *h = mpalloc(mp, hashtable);
    h->capacity = capacity;
    h->items_arr = (hashtable_node **)mpallocn(mp, capacity * sizeof(void *), "hashtable_items_arr");
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
    hashtable_node *node = mpalloc(h->mempool, hashtable_node);
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

static void *hashtable_iterator_lookahead(iterator *it, int times) {
    // not supported for now
    return NULL;
}

iterator *hashtable_create_iterator(hashtable *h, mempool *mp) {
    hashtable_iterator_private_state *it_state = mpalloc(mp, hashtable_iterator_private_state);
    it_state->hashtable = h;
    it_state->curr_slot_no = -1;
    it_state->curr_node = NULL;

    iterator *it = mpalloc(mp, iterator);
    it->private_data = it_state;
    it->reset = hashtable_iterator_reset;
    it->valid = hashtable_iterator_valid;
    it->next = hashtable_iterator_next;
    it->lookahead = hashtable_iterator_lookahead;
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

