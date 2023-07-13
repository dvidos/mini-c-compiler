#include "section.h"
#include <stdlib.h>


static void _set_name(section *s, char *name);
static void _set_address(section *s, u64 address);
static void _print(section *s);
static void _append(section *s, section *other);

static struct section_vtable vtable = {
    .set_name = _set_name,
    .set_address = _set_address,
    .print = _print,
    .append = _append,
};

section *new_section(mempool *mp) {
    section *s = mpalloc(mp, section);

    s->contents = new_bin(mp);
    s->symbols = new_symbol_table();
    s->relocations = new_reloc_list();

    s->v = &vtable;

    return s;
}

static void _set_name(section *s, char *name) {
    s->name = name == NULL ? NULL : strdup(name);
}

static void _set_address(section *s, u64 address) {
    s->address = address;
}

static void _print(section *s) {
    printf("Section %s\n", s->name == NULL ? "(null)" : s->name);
    if (bin_len(s->contents) > 0) {
        printf("  Contents (%ld bytes)\n", bin_len(s->contents));
        int max_size = bin_len(s->contents) > 128 ? 128 : bin_len(s->contents);
        bin_print_hex(s->contents, 4, 0, max_size, stdout);
        if (bin_len(s->contents) > 128)
            printf("    ... (trimmed)\n");
    }
    if (s->symbols->length > 0) {
        printf("  Symbols (%d entries)\n", s->symbols->length);
        s->symbols->print(s->symbols);
    }
    if (s->relocations->length > 0) {
        printf("  Relocations (%d entries)\n", s->relocations->length);
        s->relocations->print(s->relocations);
    }
}

static void _append(section *s, section *other) {
    long address_offset = bin_len(s->contents);
    bin_cat(s->contents, other->contents);
    
    s->symbols->append(s->symbols, other->symbols, address_offset);
    s->relocations->append(s->relocations, other->relocations, address_offset);
}

