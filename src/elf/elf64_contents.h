#pragma once
#include <stdint.h>
#include "elf_format.h"
#include "../utils/data_types.h"
#include "../utils/data_structs.h"



typedef struct elf64_contents {
    elf64_header *header;
    llist *sections;       // items are of type "elf64_section"
    llist *prog_headers;   // items are of type "elf64_prog_header"

    mempool *mempool;
} elf64_contents;

typedef struct elf64_section {
    int index; // e.g. zero section is the empty section
    str *name; // e.g. ".text"
    elf64_section_header *header;
    bin *contents;
} elf64_section;


elf64_contents *new_elf64_contents(mempool *mp);
elf64_section *new_elf64_section(mempool *mp);
elf64_header *new_elf64_file_header(mempool *mp, bool executable, u64 entry_point);
elf64_prog_header *new_elf64_prog_header(mempool *mp, int type, int flags, int align, u64 file_offset, u64 file_size, u64 mem_addr, u64 mem_size);
elf64_section_header *new_elf64_section_header(int type, char *name, u64 flags, u64 file_offset, u64 size, u64 item_size, u64 link, u64 info, u64 virt_address, bin *sections_names_table, mempool *mp);

elf64_section *elf64_get_section_by_name(elf64_contents *contents, str *name);
elf64_section *elf64_get_section_by_index(elf64_contents *contents, int index);
elf64_section *elf64_get_section_by_type(elf64_contents *contents, int type);

elf64_contents *elf64_load_file(mempool *mp, char *filename);
bool            elf64_save_file(char *filename, elf64_contents *contents);

void elf_unit_tests();

