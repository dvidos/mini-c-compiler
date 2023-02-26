#include "ref_list.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


static void ref_list_add(struct ref_list *list, u64 offset, char *name);


struct ref_list *new_ref_list() {
    struct ref_list *p = malloc(sizeof(struct ref_list));
    p->capacity = 10;
    p->references = malloc(p->capacity * sizeof(struct reference));
    p->count = 0;

    p->add = ref_list_add;

    return p;
}

static void ref_list_add(struct ref_list *list, u64 position, char *name) {
    if (list->count + 1 >= list->capacity) {
        list->capacity *= 2;
        list->references = realloc(list->references, list->capacity * sizeof(struct ref_list));
    }

    list->references[list->count].position = position;
    list->references[list->count].name = strdup(name);
    list->count++;
}
