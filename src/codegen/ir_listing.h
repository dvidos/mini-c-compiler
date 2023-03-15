#pragma once
#include <stdbool.h>
#include <stdio.h>
#include "ir_entry.h"


struct ir_listing_ops;

typedef struct ir_listing {
    ir_entry **entries_arr; // array of pointers
    int capacity;
    int length;

    struct ir_listing_ops *ops;
} ir_listing;

ir_listing *new_ir_listing();

struct ir_listing_ops {
    void (*add)(ir_listing *l, ir_entry *entry);
    void (*print)(ir_listing *l, FILE *stream);
    int (*find_next_function_def)(ir_listing *l, int start);
    void (*free)(ir_listing *l);
};

