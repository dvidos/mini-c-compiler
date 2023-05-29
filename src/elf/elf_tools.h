#pragma once
#include <stdint.h>
#include "../utils/data_types.h"
#include "../utils/data_structs.h"

typedef struct elffile {

} elffile;

struct elffile_section {
    str *name;
    binary *contents;
    u64 mem_address;
};

typedef struct elffile_symbol {
    size_t value;
    size_t size;
    enum { EST_NOTYPE = 0, EST_DATA_OBJ = 1, EST_FUNCTION = 2, EST_SECTION = 3, EST_FILE = 4, EST_COMMON = 5  } type;
    enum { ESB_LOCAL = 0, ESB_GLOBAL = 1, ESB_WEAK = 2 } bind;
    enum { DEFAULT } visibility;
    enum { UND, ABS, NUM } index_type;
    int index;
    char *name;
} elffile_symbol;

typedef struct elffile_relocation {
    size_t offset;
    size_t symbol_num;
    int type; // CPU specific, names include: R_X86_64_PC32, R_X86_64_PLT32, R_386_PC32 (plt = procedure linkage table)
    long addendum; // signed
} elffile_relocation;

// string tables should be the responsibility of the ELF save/load methods.

elffile *new_elffile(mempool *mp);
elffile *new_elffile_from_obj_file(mempool *mp, char *filename);

void elffile_print_info(elffile *e, FILE *f);
void elffile_save_obj_file(elffile *e, char *filename);
void elffile_save_executable_file(elffile *e, char *filename);

void elffile_get_symbols_count(elffile *e);
elffile_symbol *elffile_get_symbol(elffile *e, int num, mempool *mp);
void elffile_add_symbol(elffile *e, elffile_symbol *s);

void elffile_get_relocations_count(elffile *e);
elffile_relocation *elffile_get_relocation(elffile *e, int num, mempool *mp);
void elffile_add_relocation(elffile *e, elffile_relocation *reloc);







// ----------------------------------------------------------

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

typedef struct elf_symbol {
    
} elf_symbol;

typedef struct elf_relocation {
    
} elf_relocation;

elf_contents2 *new_elf_contents2(mempool *mp);
elf_contents2 *load_elf64_obj_file(mempool *mp, char *filename);

bool save_elf64_executable(char *filename, elf_contents2 *contents, u64 entry_point);
bool save_elf64_obj_file(char *filename, elf_contents2 *contents);

void elf_unit_tests();

