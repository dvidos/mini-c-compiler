#pragma once
#include <stdint.h>


typedef uint64_t u64;


enum operand_type {
    OT_NONE = 0,              // this operand is not to be used
    OT_IMMEDIATE,             // size depends
    OT_REGISTER,              // e.g. EAX
    OT_MEM_DWORD_POINTED_BY_REG, // e.g. [EAX]
    OT_SYMBOL_MEM_ADDRESS,    // e.g. address of symbol (resolved at linking)
};

// these have to be 0 through 7, in this sequence.
enum reg {
    REG_AX = 0, 
    REG_CX = 1,
    REG_DX = 2,
    REG_BX = 3,
    REG_SP = 4,
    REG_BP = 5,
    REG_SI = 6,
    REG_DI = 7
};

struct operand {
    enum operand_type type;
    union {
        long value; // register no or immediate
        char *symbol_name;
    };
    long offset; // for indirect memory access
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
    char *label;
    enum opcode opcode;
    struct operand op1;
    struct operand op2;
};

void instruction_to_string(struct instruction *inst, char *buff, int buff_size);