#pragma once
#include <stdio.h>
#include <string.h>
#include "symbol_table.h"
#include "../utils/all.h"
#include "../utils.h"
#include "reloc_list.h"


typedef struct section {
    char *name;
    u64 address;

    // code for .text, or data for .data etc
    bin *contents;

    // symbols both local and exported in this section
    symbol_table *symbols;

    // references in the code segment that need to be resolved at link time
    reloc_list *relocations;

    struct section_vtable *v;
} section;

struct section_vtable {
    void (*set_name)(section *s, char *name);
    void (*set_address)(section *s, u64 address);
    void (*print)(section *s);
    void (*append)(section *s, section *other);
};

section *new_section(mempool *mp);

