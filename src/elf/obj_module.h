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
} obj_module;

typedef struct obj_section {
    str *name;
    bin *contents;
    llist *symbols;      // item type is <obj_symbol>
    llist *relocations;  // item type is <obj_relocation>
} obj_section;

typedef struct obj_symbol {
    str *name;
    size_t value;   // essentially an address or an offset in the contents
    size_t size;    // e.g. functions size in code, or data in bytes
    bool global;    // otherwise it's just local
} obj_symbol;

typedef struct obj_relocation {
    size_t offset;      // where in the section to backfill
    str *symbol_name; 
    int type;           // CPU specific, names include: R_X86_64_PC32, R_X86_64_PLT32, R_386_PC32 (plt = procedure linkage table)
    long addendum;      // signed, as it can be positive or negative
} obj_relocation;

obj_module *new_obj_module(mempool *mp, const char *name);
obj_module *unpack_elf64_contents(str *module_name, elf64_contents *contents, mempool *mp);
elf64_contents *pack_elf64_object_file(obj_module *module, mempool *mp);
elf64_contents *pack_elf64_executable_file(obj_module *module, mempool *mp);

void print_obj_module(obj_module *module, FILE *f);

