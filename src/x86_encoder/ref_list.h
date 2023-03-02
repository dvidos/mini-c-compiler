#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "symbol_table.h"
#include "bin_buffer.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


struct reference {
    u64 position; // where the reference is to be patched (4 bytes)
    char *name;  // which symbol's value to put there
};

struct ref_list {
    struct reference *references;
    int capacity;
    int length;
    void (*add)(struct ref_list *list, u64 position, char *name);
    void (*clear)(struct ref_list *list);
    bool (*backfill_buffer)(struct ref_list *list, struct symbol_table *symbols, struct bin_buffer *buff, u64 base_address, int ref_size_bytes);
    void (*free)(struct ref_list *list);
};

struct ref_list *new_ref_list();
