#pragma once
#include <stdbool.h>
#include "../asm_line.h"



struct encoding_info {
    bool has_instruction_expansion_byte; // only a few instructions do
    unsigned char instruction_expansion_byte; // usually 0x0F;

    unsigned char base_opcode_byte; // the basis, adding direction and size to this
    bool has_width_bit;     // usually bit 0 (0=operands 1 byte, 1=operands full size, 4 bytes)
    bool has_direction_bit; // usually bit 1 (0=reg -> ModRM/SIB, 1=ModRm/SIB -> Reg)
    bool has_opcode_extension; // usually in the place of reg in ModRegRm
    char opcode_extension_value; // valid values 0-7, used often with immediates
    bool is_reg_part_of_opcode;  // e.g. PUSH EBP (0x55)

    bool needs_modregrm; // means ModRegRM (and possibly SIB) byte is needed
    bool displacement_without_modrm; // four bytes of displacement


    // type of immediate support provided: none, a fixed number of bits, or depending on the sign expanded bit
    // "sign expanded bit", usually bit 1 (not zero) of opcode, instead of direction bit
    // in means that immediate can be shortened to 1 byte, instead of four
    enum { IMM_NONE, IMM_FIXED8, IMM_FIXED32, IMM_SIGN_EXP_BIT } immediate_support;
};


bool load_encoding_info(asm_instruction *inst, struct encoding_info *info);
