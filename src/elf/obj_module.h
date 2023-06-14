#pragma once
#include <stdio.h>
#include "../utils/data_types.h"
#include "../utils/data_structs.h"
#include "elf64_contents.h"

typedef struct obj_section obj_section;


typedef struct obj_module {
    str *name; // e.g. mcc.c
    obj_section *text;
    obj_section *data;
    obj_section *bss;
    obj_section *rodata;

    struct obj_module_ops {
        void (*print)(struct obj_module *m, FILE *f);
        // elf64_contents *(*pack_object_code)(struct obj_module *m, mempool *mp);
        // elf64_contents *(*pack_executable)(struct obj_module *m, mempool *mp);
    } *ops;
} obj_module;

typedef struct obj_section {
    str *name;
    bin *contents;
    llist *symbols;      // item type is <obj_symbol>
    llist *relocations;  // item type is <obj_relocation>

    struct obj_section_ops {
        void (*print)(struct obj_section *s, FILE *f);
        // append another section
        // rebase offsets to another value
    } *ops;
} obj_section;

typedef struct obj_symbol {
    str *name;
    size_t value;   // essentially an address or an offset in the contents
    size_t size;    // e.g. functions size in code, or data in bytes
    bool global;    // otherwise it's just local

    struct obj_symbol_ops {
        void (*print)(struct obj_symbol *s, FILE *f);
    } *ops;
} obj_symbol;

typedef struct obj_relocation {
    size_t offset;      // where in the section to backfill
    str *symbol_name; 
    int type;           // CPU specific, names include: R_X86_64_PC32, R_X86_64_PLT32, R_386_PC32 (plt = procedure linkage table)
    long addendum;      // signed, as it can be positive or negative

    struct obj_relocation_ops {
        void (*print)(struct obj_relocation *r, FILE *f);
    } *ops;
} obj_relocation;


obj_module *new_obj_module(mempool *mp, const char *name);
obj_module *new_obj_module_from_elf64_contents(str *module_name, elf64_contents *contents, mempool *mp);

elf64_contents *pack_elf64_object_file(obj_module *module, mempool *mp);
elf64_contents *pack_elf64_executable_file(obj_module *module, mempool *mp);

void obj_module_print(obj_module *module, FILE *f);

