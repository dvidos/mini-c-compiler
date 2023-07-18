#pragma once
#include <stdio.h>
#include <stdbool.h>
#include "../utils/all.h"
#include "asm_line.h"

typedef struct asm_listing asm_listing;
struct asm_listing_ops;

typedef struct asm_listing {
    str *next_label;
    str *next_comment;

    mempool *mempool;
    list *lines; // item is asm_line

    struct asm_listing_ops *ops;
} asm_listing;

struct asm_listing_ops {
    void (*print)(asm_listing *lst, FILE *stream);
    void (*select_section)(asm_listing *lst, const char *name, ...);
    void (*declare_global)(asm_listing *lst, const char *name, ...);
    void (*declare_extern)(asm_listing *lst, const char *name, ...);
    void (*set_next_label)(asm_listing *lst, const char *label, ...);
    void (*set_next_comment)(asm_listing *lst, char *comment, ...);
    void (*add_comment)(asm_listing *lst, char *comment, ...);
    void (*add_line)(asm_listing *lst, asm_line *line);
};

asm_listing *new_asm_listing(mempool *mp);

