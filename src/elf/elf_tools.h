#pragma once
#include <stdint.h>
#include "../utils/data_types.h"
#include "../utils/data_structs.h"


// these are needed for symbols referring to sections
#define ELF_UNDEF_SHNDX    0 // first section is the empty section
#define ELF_TEXT_SHNDX     1
#define ELF_DATA_SHNDX     2
#define ELF_BSS_SHNDX      3
#define ELF_RODATA_SHNDX   4


typedef struct elf_contents2_section {
    bin *contents;
    u64 mem_address;
} elf_contents2_section;

// maybe we should refactor this to have a list of section headers + contents
// and a list of program headers... ? that way, we can resolve section indexes (e.g. "link")
typedef struct elf_contents2 {
    elf_contents2_section *text;      // contains raw code
    elf_contents2_section *data;      // contains initialized data
    elf_contents2_section *bss;       // contains non-initialized data
    elf_contents2_section *rodata;    // contains initialized data, to be read only
    elf_contents2_section *rela_text; // text relocations
    elf_contents2_section *rela_data; // data relocations (e.g. pointers to .rodata items)
    elf_contents2_section *symtab;    // symbols
    elf_contents2_section *strtab;    // strings table (e.g. symbol names)
    elf_contents2_section *comment;   // free text?
} elf_contents2;

elf_contents2 *new_elf_contents2(mempool *mp);
elf_contents2 *load_elf64_obj_file(mempool *mp, char *filename);

bool save_elf64_executable(char *filename, elf_contents2 *contents, u64 entry_point);
bool save_elf64_obj_file(char *filename, elf_contents2 *contents);

void elf_unit_tests();

