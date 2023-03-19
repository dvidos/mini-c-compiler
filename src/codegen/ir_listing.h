#pragma once
#include <stdbool.h>
#include <stdio.h>
#include "ir_entry.h"


struct ir_listing_ops;

typedef struct ir_listing {
    ir_entry **entries_arr; // array of pointers
    int capacity;
    int length;
    struct {
        int min_reg_no;
        int max_reg_no;
        int regs_count;
        int *reg_last_usage_arr; // array of max_index per reg_no
    } statistics;

    struct ir_listing_ops *ops;
} ir_listing;

ir_listing *new_ir_listing();

struct ir_listing_ops {
    void (*add)(ir_listing *l, ir_entry *entry);
    void (*print)(ir_listing *l, FILE *stream);
    int (*find_next_function_def)(ir_listing *l, int start);
    void (*run_statistics)(ir_listing *l);
    int (*get_register_last_usage)(ir_listing *l, int reg_no);
    void (*free)(ir_listing *l);
};

