#pragma once
#include <stdbool.h>
#include <stdint.h>


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

enum symbol_base {
    SB_CODE,
    SB_DATA,
    SB_BSS,
};

struct symbol_entry {
    char *name;  // would be smallish, but allocate for any size of name
    u64 address;  // symbols always represent an address in a segment/section.

    // elf also has binding: internal, external, weak.
    // elf also has visibility: default, internal, hidden, protected.
    // elf also has type: data object, function, section, file, etc
    // elf also has symbol size

    // a symbol should also have an indicator of the "section" it belongs to,
    // e.g. if the address is for the data, text, rodata, or bss segments
    enum symbol_base base;
};

typedef struct symbol_table symbol_table;

struct symbol_table {
    struct symbol_entry *symbols;
    int capacity;
    int length;

    void (*clear)(symbol_table *table);
    void (*add)(symbol_table *table, char *name, u64 address, enum symbol_base base);
    struct symbol_entry *(*find)(symbol_table *table, char *name);
    void (*print)(symbol_table *table);
    void (*offset)(symbol_table *table, enum symbol_base base, long offset);
    void (*append)(symbol_table *table, symbol_table *source);
    void (*free)(symbol_table *table);
};

symbol_table *new_symbol_table();
