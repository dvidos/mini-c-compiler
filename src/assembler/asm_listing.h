#pragma once
#include <stdio.h>
#include <stdbool.h>
#include "../utils/data_structs.h"
#include "asm_line.h"

typedef struct asm_listing asm_listing;
struct asm_listing_ops;

typedef struct asm_listing {
    str *next_label;
    str *next_comment;

    mempool *mempool;
    llist *lines; // item is asm_line

    struct asm_listing_ops *ops;
} asm_listing;

struct asm_listing_ops {
    void (*print)(asm_listing *lst, FILE *stream);
    void (*set_next_label)(asm_listing *lst, char *label, ...);
    void (*set_next_comment)(asm_listing *lst, char *comment, ...);
    void (*add_comment)(asm_listing *lst, char *comment, ...);
    void (*add_instruction)(asm_listing *lst, asm_instruction *instr);
};

asm_listing *new_asm_listing(mempool *mp);

