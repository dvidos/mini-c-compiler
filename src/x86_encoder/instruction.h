#pragma once
#include <stdint.h>


typedef uint64_t u64;


enum operand_type {
    OT_IMMEDIATE,  // size depends on encoder mode
    OT_REGISTER,
    OT_MEMORY_POINTED_BY_REG,
    OT_MEMORY_POINTED_BY_SYMBOL,
};

enum reg {
    REG_AX = 0, 
    REG_CX,
    REG_DX,
    REG_BX,
    REG_SP,
    REG_BP,
    REG_SI,
    REG_DI
};

struct operand {
    enum operand_type type;
    union {
        long value; // register no, immediate
        long offset;
        char *symbol_name;
    };
};

enum opcode {
    OC_NOP, 
    OC_MOV, 
    OC_PUSH, 
    OC_POP, 
    OC_LEA, // see https://stackoverflow.com/questions/1658294/whats-the-purpose-of-the-lea-instruction
    // arithmetic
    OC_ADD, 
    OC_SUB,
    OC_INC,
    OC_DEC,
    OC_IMUL,
    OC_IDIV,
    OC_AND,
    OC_OR,
    OC_XOR,
    OC_NOT,
    OC_NEG,
    OC_SHL,
    OC_SHR,
    // control
    OC_JMP,
    OC_CMP,
    OC_JEQ,
    OC_JNE,
    OC_JGT,
    OC_JGE,
    OC_JLT,
    OC_JLE,
    OC_CALL,
    OC_RET,
    OC_INT,
};


struct instruction {
    enum opcode opcode;
    struct operand op1;
    struct operand op2;
};

