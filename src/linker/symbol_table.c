#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symbol_table.h"

static void _clear(symbol_table *table);
static void _add(symbol_table *table, char *name, u64 offset, enum symbol_base base);
static struct symbol_entry *_find(symbol_table *table, char *name);
static void _print(symbol_table *table);
static void _offset(symbol_table *table, long offset);
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
    p->free = _free;

    return p;
}

static void _add(symbol_table *table, char *name, u64 offset, enum symbol_base base) {
    if (table->length + 1 >= table->capacity) {
        table->capacity *= 2;
        table->symbols = realloc(table->symbols, table->capacity * sizeof(struct symbol_entry));
    }

    struct symbol_entry *sym = &table->symbols[table->length];
    sym->name = strdup(name);
    sym->offset = offset;
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
                    s->base == SB_ZERO_DATA ? "bss" : "???"
                )
            ),
            s->offset
        );
    }
}

static void _offset(symbol_table *table, long offset) {
    for (int i = 0; i < table->length; i++) {
        table->symbols[i].offset += offset;
    }
}

static void _free(symbol_table *table) {
    free(table->symbols);
    free(table);
}
