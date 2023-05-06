#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symbol_table.h"

static void _clear(symbol_table *table);
static void _add(symbol_table *table, char *name, u64 address, enum symbol_base base);
static struct symbol_entry *_find(symbol_table *table, char *name);
static void _print(symbol_table *table);
static void _offset(symbol_table *table, enum symbol_base base, long offset);
static void _append(symbol_table *table, symbol_table *source, long address_offset);
static void _free(symbol_table *table);

symbol_table *new_symbol_table() {
    symbol_table *p = malloc(sizeof(symbol_table));
    p->capacity = 10;
    p->symbols = malloc(p->capacity * sizeof(struct symbol_entry));
    p->length = 0;

    p->add = _add;
    p->find = _find;
    p->clear = _clear;
    p->print = _print;
    p->offset = _offset;
    p->append = _append;
    p->free = _free;

    return p;
}

static void _ensure_capacity(symbol_table *table, int capacity) {
    if (table->capacity < capacity) {
        while (table->capacity < capacity)
            table->capacity *= 2;
        
        table->symbols = realloc(table->symbols, table->capacity * sizeof(struct symbol_entry));
    }
}

static void _add(symbol_table *table, char *name, u64 address, enum symbol_base base) {
    _ensure_capacity(table, table->length + 1);

    struct symbol_entry *sym = &table->symbols[table->length];
    sym->name = strdup(name);
    sym->address = address;
    sym->base = base;
    table->length++;
}

static void _clear(symbol_table *table) {
    table->length = 0;
}

static struct symbol_entry *_find(symbol_table *table, char *name) {
    for (int i = 0; i < table->length; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

static void _print(symbol_table *table) {
    printf("  Name                    Base    Offset\n");
    //     "  12345678901234567890    1234    12345678"
    for (int i = 0; i < table->length; i++) {
        struct symbol_entry *s = &table->symbols[i];
        printf("  %-20s    %4s    %08lx\n",
            s->name, 
            s->base == SB_CODE ? "code" : (
                s->base == SB_DATA ? "data" : (
                    s->base == SB_BSS ? "bss" : "???"
                )
            ),
            s->address
        );
    }
}

static void _offset(symbol_table *table, enum symbol_base base, long offset) {
    for (int i = 0; i < table->length; i++) {
        if (table->symbols[i].base == base)
            table->symbols[i].address += offset;
    }
}

static void _append(symbol_table *table, symbol_table *source, long address_offset) {
    _ensure_capacity(table, table->length + source->length);

    for (int i = 0; i < source->length; i++) {
        struct symbol_entry *src_sym = &source->symbols[i];

        struct symbol_entry *tgt_sym = &table->symbols[table->length];
        tgt_sym->name = strdup(src_sym->name);
        tgt_sym->address = src_sym->address + address_offset;
        tgt_sym->base = src_sym->base;
        table->length++;
    }
}

static void _free(symbol_table *table) {
    for (int i = 0; i < table->length; i++)
        free(table->symbols[i].name);
    free(table->symbols);
    free(table);
}
