#pragma once
#include <stdio.h>
#include <stdbool.h>
#include "asm_instruction.h"

typedef struct asm_listing asm_listing;
struct asm_listing_ops;

typedef struct asm_listing {
    char *next_label;
    char *next_comment;
    struct asm_instruction *instructions;
    int capacity;
    int length;

    struct asm_listing_ops *ops;
} asm_listing;

struct asm_listing_ops {
    void (*print)(asm_listing *lst, FILE *stream);
    void (*set_next_label)(asm_listing *lst, char *label);
    void (*add_comment)(asm_listing *lst, char *comment, bool for_next_instruction);
    void (*add_instr)(asm_listing *lst, enum opcode code); // e.g. NOP
    void (*add_instr1)(asm_listing *lst, enum opcode code, struct asm_operand *op1);
    void (*add_instr2)(asm_listing *lst, enum opcode code, struct asm_operand *op1, struct asm_operand *op2);
};

asm_listing *new_asm_listing();
struct asm_operand *new_imm_asm_operand(int value);
struct asm_operand *new_reg_asm_operand(enum reg reg_no);
struct asm_operand *new_mem_by_sym_asm_operand(char *symbol_name);
struct asm_operand *new_mem_by_reg_operand(enum reg reg_no, int offset);

