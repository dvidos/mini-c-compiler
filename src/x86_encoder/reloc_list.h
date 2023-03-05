#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "symbol_table.h"
#include "buffer.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

enum reloc_type {
    RT_ABS_32,   // just copy the address of the symbol using 4 bytes
    RT_REL_32,   // increase the value of position by the relative distance to the symbol (for code jumps)
};

struct relocation {
    u64 position; // where the relocation is to be patched (4 bytes)
    char *name;  // which symbol's value to put there
    enum reloc_type type;
    // we can also have the type of ref, (e.g. R_386_32 or R_*_GOT32, etc)
    // which says how to calculate (e.g. relative or absolute)
    // what size relocation to affect (e.g. 4 bytes vs 1 byte)
};

typedef struct reloc_list reloc_list;

struct reloc_list {
    struct relocation *list;
    int capacity;
    int length;

    void (*add)(reloc_list *list, u64 position, char *name, enum reloc_type type);
    void (*clear)(reloc_list *list);
    bool (*backfill_buffer)(reloc_list *list, symbol_table *symbols, buffer *buff, u64 code_base_address, u64 data_base_address, u64 bss_base_address);
    void (*print)(reloc_list *list);
    void (*free)(reloc_list *list);
};

struct reloc_list *new_reloc_list();
