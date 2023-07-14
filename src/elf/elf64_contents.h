#pragma once
#include <stdint.h>
#include "elf_format.h"
#include "../utils/all.h"

typedef struct elf64_contents elf64_contents;
typedef struct elf64_section elf64_section;

struct elf64_contents {
    elf64_header *header;
    list *sections;       // items are of type "elf64_section"
    list *prog_headers;   // items are of type "elf64_prog_header"

    struct elf64_contents_ops { 
        elf64_section *(*create_section)(elf64_contents *contents, str *name, size_t type);
        elf64_prog_header *(*create_prog_header)(elf64_contents *contents);
        void (*add_section)(elf64_contents *contents, elf64_section *s);
        void (*add_prog_header)(elf64_contents *contents, elf64_prog_header *p);

        elf64_section *(*get_section_by_name)(elf64_contents *contents, str *name);
        elf64_section *(*get_section_by_index)(elf64_contents *contents, int index);
        elf64_section *(*get_section_by_type)(elf64_contents *contents, int type);

        void (*print)(elf64_contents *contents, FILE *stream);
        bool (*save)(elf64_contents *contents, str *filename);
    } *ops;

    mempool *mempool;
};

struct elf64_section {
    int index; // e.g. zero section is the empty section
    str *name; // e.g. ".text"
    elf64_section_header *header;
    bin *contents;

    struct elf64_section_ops {
        void (*add_symbol)(elf64_section *s, size_t name_offset, size_t value, size_t size, int type, int binding, int section_index);
        void (*add_relocation)(elf64_section *s, size_t offset, size_t symbol_no, size_t type, long addendum);
        size_t (*add_strz_get_offset)(elf64_section *s, str *string);
        int (*find_named_symbol)(elf64_section *s, str *name, elf64_section *strtab);
        void (*add_named_symbol)(elf64_section *s, str *name, size_t value, size_t size, int type, int binding, int section_index, elf64_section *strtab);
        void (*add_named_relocation)(elf64_section *s, size_t offset, str *symbol_name, size_t type, long addend, elf64_section *symtab, elf64_section *strtab);
        void (*print)(elf64_section *s, FILE *stream);
        int (*count_symbols)(elf64_section *s);
        void (*print_symbol)(elf64_section *s, int symbol_no, elf64_section *strtab, FILE *stream);
        int (*count_relocations)(elf64_section *s);
        void (*print_relocation)(elf64_section *s, int rel_no, elf64_section *symtab, elf64_section *strtab, FILE *stream);
    } *ops;

    mempool *mempool;
};


elf64_contents *new_elf64_contents(mempool *mp);
elf64_contents *new_elf64_contents_from_binary(mempool *mp, bin *buffer);

void elf_unit_tests();

