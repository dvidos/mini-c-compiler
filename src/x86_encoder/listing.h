#pragma once
#include "instruction.h"

typedef struct listing listing;
struct listing_ops;

struct listing {
    char *next_label;
    struct instruction *instructions;
    int capacity;
    int length;

    struct listing_ops *ops;
};

struct listing_ops {
    void (*print)(listing *lst);
    void (*set_next_label)(listing *lst, char *label);
    void (*add_single_instruction)(listing *lst, enum opcode code); // e.g. NOP
    void (*add_instruction_with_immediate)(listing *lst, enum opcode code, u64 value); // e.g. PUSH 1
    void (*add_instruction_with_register_and_immediate)(listing *lst, enum opcode code, enum reg reg, u64 value);
    void (*add_instruction_with_register_and_symbol)(listing *lst, enum opcode code, enum reg reg, char *symbol_name);
    void (*add_instruction_with_register)(listing *lst, enum opcode code, enum reg reg); // e.g. PUSH EAX
};

listing *new_listing();
