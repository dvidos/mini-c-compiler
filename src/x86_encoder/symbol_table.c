#include "symbol_table.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static void _clear(symbol_table *table);
static void _add(symbol_table *table, char *name, u64 offset, enum symbol_base base);
static struct symbol *_find(symbol_table *table, char *name);
static void _free(symbol_table *table);

symbol_table *new_symbol_table() {
    symbol_table *p = malloc(sizeof(symbol_table));
    p->capacity = 10;
    p->symbols = malloc(p->capacity * sizeof(struct symbol));
    p->length = 0;

    p->add = _add;
    p->find = _find;
    p->clear = _clear;
    p->free = _free;

    return p;
}

static void _add(symbol_table *table, char *name, u64 offset, enum symbol_base base) {
    if (table->length + 1 >= table->capacity) {
        table->capacity *= 2;
        table->symbols = realloc(table->symbols, table->capacity * sizeof(struct symbol));
    }

    struct symbol *sym = &table->symbols[table->length];
    sym->name = strdup(name);
    sym->offset = offset;
    sym->base = base;
    table->length++;
}

static void _clear(symbol_table *table) {
    table->length = 0;
}

static struct symbol *_find(symbol_table *table, char *name) {
    for (int i = 0; i < table->length; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

static void _free(symbol_table *table) {
    free(table->symbols);
    free(table);
}
