#pragma once
#include <stdbool.h>
#include <stdint.h>


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct symbol {
    char *name;  // would be smallish, but allocate for any size of name
    u64 offset;  // symbols always represent an offset in a segment/section.
    // elf also has binding: internal, external, weak.
    // elf also has visibility: default, internal, hidden, protected.
    // elf also has type: data object, function, section, file, etc
    // elf also has symbol size
};

struct symbol_table {
    struct symbol *symbols;
    int capacity;
    int count;
    void (*append_symbol)(struct symbol_table *table, char *name, u64 offset);
    bool (*find_symbol)(struct symbol_table *table, char *name, u64 *p_offset);
    void (*free)(struct symbol_table *table);
};

struct symbol_table *new_symbol_table();
