#pragma once
#include <stdbool.h>
#include "asm_instruction.h"



struct encoding_info {
    bool has_instruction_expansion_byte; // only a few instructions do
    unsigned char instruction_expansion_byte; // usually 0x0F;

    unsigned char base_opcode_byte; // the basis, adding direction and size to this
    bool has_width_bit;     // usually bit 0
    bool has_direction_bit; // usually bit 1
    char opcode_extension; // valid values 0-7, -1 means N/A, used often with immediates
    bool is_reg_part_of_opcode;  // e.g. PUSH EBP (0x55)

    bool supports_immediate_value; // does in support immediate following?
    bool needs_modregrm; // means ModRegRM (and possibly SIB) byte is needed

    // usually bit 1 (not zero) of opcode, instead of direction bit
    // means that immediate can be shortened to 1 byte, instead of four
    bool has_sign_expanded_immediate_bit; 
};


bool load_encoding_info(enum opcode op, struct encoding_info *info);