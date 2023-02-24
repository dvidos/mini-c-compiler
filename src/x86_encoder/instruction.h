#pragma once
#include <stdint.h>


typedef uint64_t u64;


enum op_addressing_mode {
        OR_IMMEDIATE_VALUE,
        OR_NAMED_REGISTER,
        OR_SYMBOL_NO,
        OR_ABS_MEMORY,
        OR_REL_MEMORY,
        OR_BASE_WITH_SCALE_AND_DISPLACEMENT,
        OR_BASE_WITH_SCALE_AND_DISPLACEMENT_WITH_ADDING_BP_AND_SI_BUT_ONLY_IN_MONDAYS,
};

struct operand {
    enum op_addressing_mode addressing;
    u64 value; // immediate value, register no (0-7), relative or abs memory address, or symbol no
    char *symbol_name;

    int is_number: 1;
    int is_temp_register: 1;
    int is_named_register: 1;
    int is_memory_offset: 1;
    int is_named_symbol: 1;
    int is_pointer_dereference: 1;
    int base;
    int scale;
    int number;
};

enum opcode {
    // basic https://stackoverflow.com/questions/1658294/whats-the-purpose-of-the-lea-instruction
    OC_MOV, OC_PUSH, OC_POP, OC_LEA,
    // arithmetic
    OC_ADD, OC_SUB, OC_INC, OC_DEC, OC_IMUL, OC_IDIV,
    OC_AND, OC_OR, OC_XOR, OC_NOT, OC_NEG,
    OC_SHL, OC_SHR,
    // control
    OC_JMP,
    OC_CMP, OC_JEQ, OC_JNE, OC_JGT, OC_JGE, OC_JLT, OC_JLE,
    OC_CALL, OC_RET
};


struct instruction {
    enum opcode opcode;
    struct operand op1;
    struct operand op2;
};

