#pragma once
#include <stdio.h>
#include "instruction.h"

typedef struct asm_listing asm_listing;
struct asm_listing_ops;

typedef struct asm_listing {
    char *next_label;
    struct instruction *instructions;
    int capacity;
    int length;

    struct asm_listing_ops *ops;
} asm_listing;

struct asm_listing_ops {
    void (*print)(asm_listing *lst, FILE *stream);
    void (*set_next_label)(asm_listing *lst, char *label);
    void (*add_single_instruction)(asm_listing *lst, enum opcode code); // e.g. NOP
    void (*add_instruction_with_immediate)(asm_listing *lst, enum opcode code, u64 value); // e.g. PUSH 1
    void (*add_instruction_with_register_and_immediate)(asm_listing *lst, enum opcode code, enum reg reg, u64 value);
    void (*add_instruction_with_register_and_symbol)(asm_listing *lst, enum opcode code, enum reg reg, char *symbol_name);
    void (*add_instruction_with_register)(asm_listing *lst, enum opcode code, enum reg reg); // e.g. PUSH EAX
};

asm_listing *new_asm_listing();
