#include "symbol_table.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static void symbol_table_append_symbol(struct symbol_table *table, char *name, u64 offset);
static bool symbol_table_find_symbol(struct symbol_table *table, char *name, u64 *offset);


struct symbol_table *new_symbol_table(int capacity) {
    struct symbol_table *p = malloc(sizeof(struct symbol_table));
    p->capacity = capacity < 10 ? 10 : capacity;
    p->symbols = malloc(p->capacity * sizeof(struct symbol *));
    p->count = 0;

    p->append_symbol = symbol_table_append_symbol;
    p->find_symbol = symbol_table_find_symbol;

    return p;
}


static void symbol_table_append_symbol(struct symbol_table *table, char *name, u64 offset) {
    if (table->count + 1 >= table->capacity) {
        table->capacity *= 2;
        table->symbols = realloc(table->symbols, table->capacity * sizeof(struct symbol *));
    }

    struct symbol *s = malloc(sizeof(struct symbol));
    s->name = strdup(name);
    s->offset = offset;

    table->symbols[table->count] = s;
    table->count++;
}

static bool symbol_table_find_symbol(struct symbol_table *table, char *name, u64 *offset) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i]->name, name) == 0) {
            if (offset != NULL)
                *offset = table->symbols[i]->offset;
            return true;
        }
    }
    return false;
}

