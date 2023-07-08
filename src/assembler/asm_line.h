#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "../utils/data_structs.h"

typedef struct asm_line asm_line;
typedef struct asm_named_definition asm_named_definition;
typedef struct asm_data_definition asm_data_definition;
typedef struct asm_instruction asm_instruction;
typedef struct asm_operand asm_operand;

enum asm_line_type {
    ALT_EMPTY,     // e.g. empty line or single comment
    ALT_SECTION,  // e.g. ".section data"
    ALT_EXTERN,   // e.g. ".extern <name>"
    ALT_GLOBAL,   // e.g. ".global <name>"
    ALT_DATA,     // e.g. "<name:> db, dw, dd, dq <name> value [, value [,...]]"
    ALT_INSTRUCTION,  // MOV RAX, 0x1234
};

// e.g. see https://en.wikibooks.org/wiki/X86_Assembly/NASM_Syntax#Hello_World_(Linux)
struct asm_line {
    str *label;
    str *comment;
    enum asm_line_type type;
    union {
        asm_named_definition *named_definition;
        asm_data_definition *data_definition;
        asm_instruction *instruction;
    } per_type;
};

// for section name, .global, .extern etc
struct asm_named_definition {
    str *name;
};

enum data_size {
    DATA_BYTE,
    DATA_WORD,
    DATA_DWORD,
    DATA_QWORD
};

struct asm_data_definition {
    str *name;
    int length_units;     // in units e.g. 24 bytes, 3 words etc.
    enum data_size unit_size;  // e.g. operand size (used to define scale in ModRmReg)
    int length_bytes;     // in bytes
    bin *initial_value;   // in bytes
};

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

struct asm_instruction {
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
        bool is_mem_addr_by_symbol; // i.e. displacement only, no regs
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
};

char *gp_reg_name(enum gp_reg r); // don't free the returned string
char *opcode_name(enum opcode code); // don't free the returned string

asm_operand *new_asm_operand_imm(int value);
asm_operand *new_asm_operand_reg(enum gp_reg reg_no);
asm_operand *new_asm_operand_mem_by_sym(char *symbol_name);
asm_operand *new_asm_operand_mem_by_reg(enum gp_reg reg_no, int offset);

asm_instruction *new_asm_instruction(enum opcode op);
asm_instruction *new_asm_instruction_with_operand(enum opcode op, asm_operand *target);
asm_instruction *new_asm_instruction_with_operands(enum opcode op, asm_operand *target, asm_operand *source);
asm_instruction *new_asm_instruction_for_reserving_stack_space(int size);
asm_instruction *new_asm_instruction_for_register(enum opcode op, enum gp_reg gp_reg);
asm_instruction *new_asm_instruction_for_registers(enum opcode op, enum gp_reg target_reg, enum gp_reg source_reg);

void asm_instruction_to_str(asm_instruction *instr, str *str, bool with_comment);



