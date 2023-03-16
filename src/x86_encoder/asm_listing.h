#pragma once
#include <stdio.h>
#include <stdbool.h>
#include "instruction.h"

typedef struct asm_listing asm_listing;
struct asm_listing_ops;

typedef struct asm_listing {
    char *next_label;
    char *next_comment;
    struct instruction *instructions;
    int capacity;
    int length;

    struct asm_listing_ops *ops;
} asm_listing;

struct asm_listing_ops {
    void (*print)(asm_listing *lst, FILE *stream);
    void (*set_next_label)(asm_listing *lst, char *label);
    void (*add_comment)(asm_listing *lst, char *comment, bool for_next_instruction);
    void (*add_instr)(asm_listing *lst, enum opcode code); // e.g. NOP
    void (*add_instr_imm)(asm_listing *lst, enum opcode code, u64 value); // e.g. PUSH 1
    void (*add_instr_reg)(asm_listing *lst, enum opcode code, enum reg reg); // e.g. PUSH EAX
    void (*add_instr_sym)(asm_listing *lst, enum opcode code, char *symbol); // e.g. CALL func1
    void (*add_instr_reg_reg)(asm_listing *lst, enum opcode code, enum reg reg1, enum reg reg2);
    void (*add_instr_reg_imm)(asm_listing *lst, enum opcode code, enum reg reg, u64 value);
    void (*add_instr_reg_sym)(asm_listing *lst, enum opcode code, enum reg reg, char *symbol_name);
    void (*add_instr_imm_reg)(asm_listing *lst, enum opcode code, u64 value, enum reg reg);
    void (*add_instr_sym_reg)(asm_listing *lst, enum opcode code, char *symbol_name, enum reg reg);
};

asm_listing *new_asm_listing();
