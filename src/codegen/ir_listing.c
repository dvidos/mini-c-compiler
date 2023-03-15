#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "ir_listing.h"


static void _add(ir_listing *l, ir_entry *entry);
static void _print(ir_listing *l, FILE *stream);
static int _find_next_function_def(ir_listing *l, int start);
static void _free(ir_listing *l);

static struct ir_listing_ops ops = {
    .add = _add,
    .print = _print,
    .find_next_function_def = _find_next_function_def,
    .free = _free
};

ir_listing *new_ir_listing() {
    ir_listing *l = malloc(sizeof(ir_listing));
    l->capacity = 10;
    l->length = 0;
    l->entries_arr = malloc(sizeof(ir_listing *) * l->capacity);
    l->ops = &ops;
    return l;
}

static void _add(ir_listing *l, ir_entry *entry) {
    if (l->length + 1 >= l->capacity) {
        l->capacity *= 2;
        l->entries_arr = realloc(l->entries_arr, sizeof(ir_listing *) * l->capacity);
    }

    l->entries_arr[l->length] = entry;
    l->length++;
}

static void _print(ir_listing *l ,FILE *stream) {
    for (int i = 0; i < l->length; i++) {
        ir_entry *e = l->entries_arr[i];
        e->ops->print(e, stream);
        fprintf(stream, "\n");
    }
}

static int _find_next_function_def(ir_listing *l, int start) {
    for (int i = start; i < l->length; i++) {
        if (l->entries_arr[i]->type == IR_FUNCTION_DEFINITION)
            return i;
    }

    return -1;
}

static void _free(ir_listing *l) {
    for (int i = 0; i < l->length; i++) {
        ir_entry *e = l->entries_arr[i];
        e->ops->free(e);
    }
    free(l->entries_arr);
    free(l);
}
