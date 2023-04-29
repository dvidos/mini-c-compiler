#include "../linker/reloc_list.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static void _add(reloc_list *list, u64 address, char *name, enum reloc_type type);
static void _clear(reloc_list *list);
static bool _backfill_buffer(reloc_list *list, symbol_table *symbols, buffer *buff);
static void _print(reloc_list *list);
static void _offset(reloc_list *list, long offset);
static void _append(reloc_list *list, reloc_list *source);
static void _free(reloc_list *list);


reloc_list *new_reloc_list() {
    reloc_list *p = malloc(sizeof(reloc_list));
    p->capacity = 10;
    p->list = malloc(p->capacity * sizeof(struct relocation));
    p->length = 0;

    p->add = _add;
    p->clear = _clear;
    p->backfill_buffer = _backfill_buffer;
    p->print = _print;
    p->offset = _offset;
    p->append = _append;
    p->free = _free;

    return p;
}

static void _ensure_capacity(reloc_list *list, int capacity) {
    if (list->capacity < capacity) {
        while (list->capacity < capacity)
            list->capacity *= 2;
        
        list->list = realloc(list->list, list->capacity * sizeof(struct relocation));
    }
}
static void _add(reloc_list *list, u64 position, char *name, enum reloc_type type) {
    _ensure_capacity(list, list->length + 1);

    list->list[list->length].position = position;
    list->list[list->length].name = strdup(name);
    list->list[list->length].type = type;
    list->length++;
}

static void _clear(reloc_list *list) {
    list->length = 0;
}

static bool _backfill_buffer(reloc_list *list, symbol_table *symbols, buffer *buff) {
    for (int i = 0; i < list->length; i++) {
        struct relocation *r = &list->list[i];

        if (r->position >= buff->length) {
            printf("Reference at byte %ld, but buffer length only %d\n", r->position, buff->length);
            return false;
        }

        struct symbol_entry *sym;
        sym = symbols->find(symbols, r->name);
        if (sym == NULL) {
            printf("Symbol not found: \"%s\"\n", r->name);
            return false;
        }

        // we are supposed to respect the relocation type.
        if (r->type == RT_ABS_32) {
            void *pos = &buff->buffer[r->position];
            *(u32 *)pos = (u32)sym->address;
        } else {
            printf("Not supported relocation type %d\n", r->type);
            return false;
        }
    }
    return true;
}

static void _print(reloc_list *list) {
    printf("  Position    Type      Name\n");
    //     "  00000000    XXXXXX    XCZXCVzxcvxvxcv....
    for (int i = 0; i < list->length; i++) {
        struct relocation *r = &list->list[i];
        printf("  %08lx    %-6s    %s\n",
            r->position, 
            r->type == RT_ABS_32 ? "ABS_32" : (
                r->type == RT_REL_32 ? "REL_32" : "???"
            ),
            r->name
        );
    }
}

static void _offset(reloc_list *list, long offset) {
    for (int i = 0; i < list->length; i++) {
        list->list[i].position += offset;
    }
}

static void _append(reloc_list *list, reloc_list *source) {
    _ensure_capacity(list, list->length + source->length);

    for (int i = 0; i < source->length; i++) {
        struct relocation *src_rel = &source->list[i];

        // set it at the end of the array
        struct relocation *tgt_rel = &list->list[list->length];
        tgt_rel->name = strdup(src_rel->name);
        tgt_rel->position = src_rel->position;
        tgt_rel->type = src_rel->type;
        list->length++;
    }
}

static void _free(reloc_list *list) {
    for (int i = 0; i < list->length; i++)
        free(list->list[i].name);
    free(list->list);
    free(list);
}
