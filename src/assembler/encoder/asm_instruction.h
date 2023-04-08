#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "../../utils/string.h"


enum operand_type {
    OT_NONE = 0,              // this operand is not to be used
    OT_IMMEDIATE,             // size depends
    OT_REGISTER,              // e.g. EAX
    OT_MEM_POINTED_BY_REG,    // e.g. [EBP+0], also offset
    OT_MEM_OF_SYMBOL,         // e.g. address of symbol (resolved at linking)
};

// these have to be 0 through 7, in this sequence.
enum gp_reg {
    REG_AX = 0, 
    REG_CX = 1,
    REG_DX = 2,
    REG_BX = 3,
    REG_SP = 4,
    REG_BP = 5,
    REG_SI = 6,
    REG_DI = 7
};

struct asm_operand {
    enum operand_type type;
    long immediate;
    enum gp_reg reg;
    char *symbol_name;
    long offset; // for indirect memory access, through BP
};

enum opcode {
    OC_NONE = 0,
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
    OC_MUL,
    OC_DIV,
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
    OC_JAB,
    OC_JAE,
    OC_JBL,
    OC_JBE,
    OC_JGT,
    OC_JGE,
    OC_JLT,
    OC_JLE,
    OC_CALL,
    OC_RET,
    OC_INT,
};

typedef struct asm_instruction {
    char *label;
    enum opcode operation; // ADD, SUB, etc. no sign cognizance
    int operands_size_bits; // 8,16,32,64 (width bit for 8bits, 0x66 prefix for 16bits)
    bool direction_op1_to_op2; // if false, the opposite
    char *comment;

    // register or memory
    // goes to the Mod+R/M part of the ModRegRM byte
    struct operand1 { 
        bool is_register;
        bool is_memory_by_reg;
        bool is_memory_by_displacement; // i.e. displacement only, no regs
        union {
            enum gp_reg reg;
            struct {
                enum gp_reg pointer_reg;
                long displacement;   // 0 means no displacement
                enum gp_reg array_index_reg;
                int array_item_size; // must be 1,2,4 or 8 for SID to be used
                char *displacement_symbol_name; // symbol will resolve to a 32-bit address
            } mem;
        } per_type;
    } operand1; 

    // register or immediate.
    // goes to the Reg part of the Mod+Reg+RM byte
    struct operand2 {  
        bool is_immediate;
        bool is_register;
        union {
            long immediate;
            enum gp_reg reg;
        } per_type;
    } operand2; 
} asm_instruction;


struct asm_instruction_old {
    char *label;
    enum opcode opcode;
    struct asm_operand *op1;
    struct asm_operand *op2;
    char *comment;
};

void instruction_old_to_string(struct asm_instruction_old *inst, char *buff, int buff_size);
char *gp_reg_name(enum gp_reg r); // don't free the returned string
char *opcode_name(enum opcode code); // don't free the returned string

asm_instruction *new_asm_instruction(enum opcode op);
asm_instruction *new_asm_instruction_with_operand(enum opcode op, struct asm_operand *target);
asm_instruction *new_asm_instruction_with_operands(enum opcode op, struct asm_operand *target, struct asm_operand *source);
asm_instruction *new_asm_instruction_for_reserving_stack_space(int size);
asm_instruction *new_asm_instruction_for_register(enum opcode op, enum gp_reg gp_reg);
asm_instruction *new_asm_instruction_for_registers(enum opcode op, enum gp_reg target_reg, enum gp_reg source_reg);

void asm_instruction_to_str(asm_instruction *instr, string *str, bool with_comment);

