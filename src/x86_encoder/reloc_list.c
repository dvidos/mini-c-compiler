#include "reloc_list.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static void _add(struct reloc_list *list, u64 offset, char *name, enum reloc_type type);
static void _clear(struct reloc_list *list);
static bool _backfill_buffer(struct reloc_list *list, struct symbol_table *symbols, struct bin_buffer *buff, u64 sym_base_address);
static void _free(struct reloc_list *list);


struct reloc_list *new_reloc_list() {
    struct reloc_list *p = malloc(sizeof(struct reloc_list));
    p->capacity = 10;
    p->list = malloc(p->capacity * sizeof(struct relocation));
    p->length = 0;

    p->add = _add;
    p->clear = _clear;
    p->backfill_buffer = _backfill_buffer;
    p->free = _free;

    return p;
}

static void _add(struct reloc_list *list, u64 position, char *name, enum reloc_type type) {
    if (list->length + 1 >= list->capacity) {
        list->capacity *= 2;
        list->list = realloc(list->list, list->capacity * sizeof(struct reloc_list));
    }

    list->list[list->length].position = position;
    list->list[list->length].name = strdup(name);
    list->list[list->length].type = type;
    list->length++;
}

static void _clear(struct reloc_list *list) {
    list->length = 0;
}

static bool _backfill_buffer(struct reloc_list *list, struct symbol_table *symbols, struct bin_buffer *buff, u64 sym_base_address) {
    for (int i = 0; i < list->length; i++) {
        struct relocation *r = &list->list[i];

        u64 symbol_address = 0;
        if (!symbols->find_symbol(symbols, r->name, &symbol_address)) {
            printf("Symbol not found: \"%s\"\n", r->name);
            return false;
        }
        
        if (r->position >= buff->length) {
            printf("Reference at byte %ld, but buffer length only %d\n", r->position, buff->length);
            return false;
        }

        // we are supposed to respect the relocation type.
        if (r->type == RT_ABS_32) {
            void *pos = &buff->data[r->position];
            *(u32 *)pos = (u32)(symbol_address + sym_base_address);
        } else {
            printf("Not supported relocation type %d\n", r->type);
            return false;
        }
    }
}

static void _free(struct reloc_list *list) {
    free(list->list);
    free(list);
}
