#pragma once
#include <stdbool.h>
#include <stdint.h>


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

enum symbol_type {
    ST_UNKNOWN = 0,
    ST_OBJECT,   // i.e. data item
    ST_FUNCTION, // i.e. a code item
    ST_SECTION,  // i.e. a section offset
    ST_FILE,
};

struct symbol_entry {
    char *name;
    u64 address; // symbols always represent an address in a segment/section.
    u64 size;    // some symbols have size
    enum symbol_type type;
    bool global; // otherwise constrained to the module

    // elf also has binding: internal, external, weak.
    // elf also has visibility: default, internal, hidden, protected.
    // elf also has type: data object, function, section, file, etc
    // elf also has symbol size
};

typedef struct symbol_table symbol_table;

struct symbol_table {
    struct symbol_entry *symbols;
    int capacity;
    int length;

    void (*clear)(symbol_table *table);
    void (*add)(symbol_table *table, char *name, u64 address, u64 size, enum symbol_type type, bool global);
    struct symbol_entry *(*find)(symbol_table *table, char *name);
    void (*print)(symbol_table *table);
    void (*offset)(symbol_table *table, long offset);
    void (*append)(symbol_table *table, symbol_table *source, long address_offset);
    void (*free)(symbol_table *table);
};

symbol_table *new_symbol_table();
