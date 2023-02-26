#include "ref_list.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


static void ref_list_add(struct ref_list *list, u64 offset, char *name);
static void ref_list_clear(struct ref_list *list);
static void ref_list_free(struct ref_list *list);


struct ref_list *new_ref_list() {
    struct ref_list *p = malloc(sizeof(struct ref_list));
    p->capacity = 10;
    p->references = malloc(p->capacity * sizeof(struct reference));
    p->length = 0;

    p->add = ref_list_add;
    p->clear = ref_list_clear;
    p->free = ref_list_free;

    return p;
}

static void ref_list_add(struct ref_list *list, u64 position, char *name) {
    if (list->length + 1 >= list->capacity) {
        list->capacity *= 2;
        list->references = realloc(list->references, list->capacity * sizeof(struct ref_list));
    }

    list->references[list->length].position = position;
    list->references[list->length].name = strdup(name);
    list->length++;
}

static void ref_list_clear(struct ref_list *list) {
    list->length = 0;
}

static void ref_list_free(struct ref_list *list) {
    free(list->references);
    free(list);
}
