#pragma once
#include "asm_line.h"

enum operand_type {
    NONE = 0,
    REG_8,  REG_16,  REG_32,  REG_64,
    MEM_8,  MEM_16,  MEM_32,  MEM_64,
    IMM_8,  IMM_16,  IMM_32,  IMM_64,
    REL_8,  REL_16,  REL_32,  REL_64,
    // some instructions favor the AX family
    REG_AL,  REG_AX,  REG_EAX,  REG_RAX, 
    // all segment registers 16 bits
    REG_CS,  REG_DS,  REG_SS,   REG_ES,   REG_FS,  REG_GS, 
    // CL is used in shift operations
    REG_CL, 
};

struct asm_instruction_encoding_info {
    // how to identify the instruction
    enum opcode opcode;
    enum operand_type op1type;
    enum operand_type op2type;

    // how to apply the encoding
    char *op_en;     // e.g. "MR" or "ZO"
    char *encoding;  // e.g. "REX.W + 89 /r"
};

struct asm_instruction_encoding_info x64_encoding_infos[] = {
};

struct asm_instruction_encoding_info compat_encoding_infos[] = {

};




// how to use this table?