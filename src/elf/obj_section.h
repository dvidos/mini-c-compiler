#pragma once
#include <stdio.h>
#include "../utils/data_types.h"
#include "../utils/data_structs.h"
#include "elf64_contents.h"


typedef struct obj_section    obj_section;
typedef struct obj_symbol     obj_symbol;
typedef struct obj_relocation obj_relocation;


struct obj_section {
    str *name;           // e.g. ".text"
    bin *contents;       // binary contents
    llist *symbols;      // item type is <obj_symbol>
    llist *relocations;  // item type is <obj_relocation>

    struct obj_section_ops {
        void (*print)(obj_section *s, FILE *f);
        void (*append)(obj_section *s, obj_section *other);
        void (*relocate)(obj_section *s, long delta);
        obj_symbol *(*find_symbol)(obj_section *s, str *name, bool exported);
    } *ops;

    mempool *mempool;
};


struct obj_symbol {
    str *name;
    size_t value;   // essentially an address or an offset in the contents
    size_t size;    // e.g. functions size in code, or data in bytes
    bool global;    // otherwise it's just local (what about weak symbols?)

    struct obj_symbol_ops {
        void (*print)(obj_symbol *s, int num, FILE *f);
    } *ops;
};


struct obj_relocation {
    size_t offset;      // where in the section to backfill
    str *symbol_name; 
    int type;           // CPU specific, names include: R_X86_64_PC32, R_X86_64_PLT32, R_386_PC32 (plt = procedure linkage table)
    long addendum;      // signed, as it can be positive or negative

    struct obj_relocation_ops {
        void (*print)(obj_relocation *r, FILE *f);
    } *ops;
};


obj_section *new_obj_section(mempool *mp);

