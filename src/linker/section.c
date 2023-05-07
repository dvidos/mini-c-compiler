#include "section.h"
#include <stdlib.h>


static void _set_name(section *s, char *name);
static void _set_address(section *s, u64 address);
static void _print(section *s);
static void _append(section *s, section *other);
static void _free(section *s);

static struct section_vtable vtable = {
    .set_name = _set_name,
    .set_address = _set_address,
    .print = _print,
    .append = _append,
    .free = _free
};

section *new_section() {
    section *s = malloc(sizeof(section));
    memset(s, 0, sizeof(section));

    s->contents = new_buffer();
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
    if (s->contents->length > 0) {
        printf("  Contents (%d bytes)\n", s->contents->length);
        int max_size = s->contents->length > 128 ? 128 : s->contents->length;
        print_16_hex(s->contents->buffer, max_size, 4);
        if (s->contents->length > 128)
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
    long address_offset = s->contents->length;
    s->contents->append(s->contents, other->contents);
    
    s->symbols->append(s->symbols, other->symbols, address_offset);
    s->relocations->append(s->relocations, other->relocations, address_offset);
}

static void _free(section *s) {
    if (s->name != NULL)
        free(s->name);
    s->contents->free(s->contents);
    s->symbols->free(s->symbols);
    s->relocations->free(s->relocations);
    free(s);
}

