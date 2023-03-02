#include "ref_list.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static void ref_list_add(struct ref_list *list, u64 offset, char *name);
static void ref_list_clear(struct ref_list *list);
static bool backfill_buffer(struct ref_list *list, struct symbol_table *symbols, struct bin_buffer *buff, u64 sym_base_address, int ref_size_bytes);
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

static bool backfill_buffer(struct ref_list *list, struct symbol_table *symbols, struct bin_buffer *buff, u64 sym_base_address, int ref_size_bytes) {
    for (int i = 0; i < list->length; i++) {
        struct reference *r = &list->references[i];

        u64 symbol_address = 0;
        if (!symbols->find_symbol(symbols, r->name, &symbol_address)) {
            printf("Symbol not found: \"%s\"\n", r->name);
            return false;
        }
        
        if (r->position >= buff->length) {
            printf("Reference at byte %ld, but buffer length only %d\n", r->position, buff->length);
            return false;
        }

        void *pos = &buff->data[r->position];
        if (ref_size_bytes == 1)
            *(u8 *)pos = (u8)(symbol_address + sym_base_address);
        else if (ref_size_bytes == 2)
            *(u16 *)pos = (u16)(symbol_address + sym_base_address);
        else if (ref_size_bytes == 4)
            *(u32 *)pos = (u32)(symbol_address + sym_base_address);
        else if (ref_size_bytes == 8)
            *(u64 *)pos = (u64)(symbol_address + sym_base_address);
        else {
            printf("Supported references sizes are 1, 2, 4 or 8 bytes, not %d\n", ref_size_bytes);
            return false;
        }
    }
}

static void ref_list_free(struct ref_list *list) {
    free(list->references);
    free(list);
}
