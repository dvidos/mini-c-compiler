#pragma once
#include <stdio.h>
#include <string.h>
#include "symbol_table.h"
#include "../utils/buffer.h"
#include "../utils.h"
#include "reloc_list.h"


typedef struct section {
    char *name;

    // code for .text, or data for .data etc
    buffer *contents;

    // symbols both local and exported in this section
    symbol_table *symbols;

    // references in the code segment that need to be resolved at link time
    reloc_list *relocations;

    struct section_vtable *v;
} section;

struct section_vtable {
    void (*set_name)(section *s, char *name);
    void (*print)(section *s);
    void (*append)(section *s, section *other);
    void (*free)(section *s);
};

section *new_section();

