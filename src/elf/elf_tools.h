#pragma once
#include <stdint.h>
#include "../utils/data_types.h"
#include "../utils/data_structs.h"


typedef uint64_t u64;

typedef struct elf_section {
    binary *contents;
    u64 mem_address;
} elf_section;

typedef struct elf_contents2 {
    elf_section *text;      // contains raw code
    elf_section *data;      // contains initialized data
    elf_section *bss;       // contains non-initialized data
    elf_section *rodata;    // contains initialized data, to be read only
    elf_section *rela_text; // text relocations
    elf_section *symtab;    // symbols
    elf_section *strtab;    // strings table (e.g. symbol names)
    elf_section *comment;   // free text?
} elf_contents2;


elf_contents2 *new_elf_contents2(mempool *mp);
elf_contents2 *load_elf64_obj_file(mempool *mp, char *filename);

bool save_elf64_executable(char *filename, elf_contents2 *contents, u64 entry_point);
bool save_elf64_obj_file(char *filename, elf_contents2 *contents);

void elf_unit_tests();

