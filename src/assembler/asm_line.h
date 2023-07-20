#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "../utils/all.h"

typedef struct asm_line            asm_line;
typedef struct asm_directive       asm_directive;
typedef struct asm_data_definition asm_data_definition;
typedef struct asm_instruction     asm_instruction;
typedef struct asm_operand         asm_operand;

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
        asm_directive *named_definition;
        asm_data_definition *data_definition;
        asm_instruction *instruction;
    } per_type;
};

// for section name, .global, .extern etc
struct asm_directive {
    str *name;
};

typedef enum data_size {
    DATA_BYTE,
    DATA_WORD,
    DATA_DWORD,
    DATA_QWORD
} data_size;

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
typedef enum gp_register {
    REG_AX = 0, 
    REG_CX = 1,
    REG_DX = 2,
    REG_BX = 3,
    REG_SP = 4,
    REG_BP = 5,
    REG_SI = 6,
    REG_DI = 7,
    REG_R8 = 8,
    REG_R9 = 9,
    REG_R10 = 10,
    REG_R11 = 11,
    REG_R12 = 12,
    REG_R13 = 13,
    REG_R14 = 14,
    REG_R15 = 15,
} gp_register;

struct asm_operand {
    enum operand_type type;
    long immediate;
    gp_register reg;
    const char *symbol_name; // for address of a symbol
    long offset;             // for indirect memory access, through BP
};

typedef enum instr_code {
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
} instr_code;


// register or memory - goes to the Mod+R/M part of the ModRegRM byte
typedef struct asm_reg_or_mem_operand {
    bool is_register;
    bool is_memory_by_reg;
    bool is_mem_addr_by_symbol; // i.e. displacement only, no regs
    union {
        gp_register reg;
        struct {
            gp_register pointer_reg;
            long displacement;   // 0 means no displacement
            gp_register array_index_reg;
            int array_item_size; // must be 1,2,4 or 8 for SID to be used
            char *displacement_symbol_name; // symbol will resolve to a 32-bit address
        } mem;
    } per_type;
} asm_reg_or_mem_operand;

typedef struct asm_reg_or_imm_operand {
    bool is_register;
    bool is_immediate;
    union {
        long immediate;
        gp_register reg;
    } per_type;
} asm_reg_or_imm_operand;

struct asm_instruction {
    instr_code operation; // ADD, SUB, etc. no sign cognizance
    int operands_size_bits; // 8,16,32,64 (width bit for 8bits, 0x66 prefix for 16bits)
    bool direction_rm_to_ri_operands; // if false, the opposite
    
    // register or memory - goes to the Mod+R/M part of the ModRegRM byte
    asm_reg_or_mem_operand regmem_operand; 

    // register or immediate - reg goes to the Reg part of the Mod+Reg+RM byte
    asm_reg_or_imm_operand regimm_operand; 
};

asm_line *new_asm_line_empty(mempool *mp);
asm_line *new_asm_line_directive_section(mempool *mp, str *section_name);
asm_line *new_asm_line_directive_extern(mempool *mp, str *symbol_name);
asm_line *new_asm_line_directive_global(mempool *mp, str *symbol_name);
asm_line *new_asm_line_data_definition(mempool *mp, str *symbol_name, data_size unit_size, size_t units, bin *initial_data);
asm_line *new_asm_line_instruction(mempool *mp, instr_code op);
asm_line *new_asm_line_instruction_with_operand(mempool *mp, instr_code op, asm_operand *target);
asm_line *new_asm_line_instruction_with_operands(mempool *mp, instr_code op, asm_operand *target, asm_operand *source);
asm_line *new_asm_line_instruction_for_reserving_stack_space(mempool *mp, int size);
asm_line *new_asm_line_instruction_for_register(mempool *mp, instr_code op, gp_register gp_reg);
asm_line *new_asm_line_instruction_for_registers(mempool *mp, instr_code op, gp_register target_reg, gp_register source_reg);

asm_operand *new_asm_operand_imm(mempool *mp, int value);
asm_operand *new_asm_operand_reg(mempool *mp, gp_register reg_no);
asm_operand *new_asm_operand_mem_by_sym(mempool *mp, char *symbol_name);
asm_operand *new_asm_operand_mem_by_reg(mempool *mp, gp_register reg_no, int offset);

char *gp_reg_name(gp_register r); // don't free the returned string
char *instr_code_name(instr_code code); // don't free the returned string
void asm_instruction_to_str(asm_instruction *instr, str *str, bool with_comment);
str *asm_line_to_str(mempool *mp, asm_line *line);



