#include "../linker/reloc_list.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static void _add(reloc_list *list, u64 offset, char *name, enum reloc_type type);
static void _clear(reloc_list *list);
static bool _backfill_buffer(reloc_list *list, symbol_table *symbols, buffer *buff, u64 code_base_address, u64 data_base_address, u64 bss_base_address);
static void _print(reloc_list *list);
static void _offset(reloc_list *list, long offset);
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
    p->free = _free;

    return p;
}

static void _add(reloc_list *list, u64 position, char *name, enum reloc_type type) {
    if (list->length + 1 >= list->capacity) {
        list->capacity *= 2;
        list->list = realloc(list->list, list->capacity * sizeof(reloc_list));
    }

    list->list[list->length].position = position;
    list->list[list->length].name = strdup(name);
    list->list[list->length].type = type;
    list->length++;
}

static void _clear(reloc_list *list) {
    list->length = 0;
}

static bool _backfill_buffer(reloc_list *list, symbol_table *symbols, buffer *buff, u64 code_base_address, u64 data_base_address, u64 bss_base_address) {
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
        
        // decide what is our base address
        u64 base_address;
        if (sym->base == SB_CODE)
            base_address = code_base_address;
        else if (sym->base == SB_DATA)
            base_address = data_base_address;
        else if (sym->base == SB_ZERO_DATA)
            base_address = bss_base_address;
        else {
            printf("Unknown base for symbol \"%s\"\n", r->name);
            return false;
        }

        // we are supposed to respect the relocation type.
        if (r->type == RT_ABS_32) {
            void *pos = &buff->buffer[r->position];
            *(u32 *)pos = (u32)(base_address + sym->offset);
        } else {
            printf("Not supported relocation type %d\n", r->type);
            return false;
        }
    }
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

static void _free(reloc_list *list) {
    free(list->list);
    free(list);
}
