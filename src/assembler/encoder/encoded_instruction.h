#pragma once
#include "../../utils/buffer.h"
#include "../../utils/str.h"


// represents encoded bytes, eases encoding & debugging
typedef struct encoded_instruction {
    struct {
        unsigned char have_instruction_prefix : 1;
        unsigned char have_address_size_prefix : 1;
        unsigned char have_operand_size_prefix : 1;
        unsigned char have_segment_override_prefix : 1;
        unsigned char have_opcode_expansion_byte: 1;
        unsigned char have_second_opcode_byte: 1;
        unsigned char have_modregrm: 1;
        unsigned char have_sib: 1;
    } flags;
    unsigned char instruction_prefix;
    unsigned char address_size_prefix;
    unsigned char operand_size_prefix;
    unsigned char segment_override_prefix;
    unsigned char opcode_expansion_byte;
    unsigned char opcode_byte; // always there
    unsigned char modregrm_byte;
    unsigned char sib_byte;
    unsigned char displacement[4];
    unsigned char displacement_bytes_count; // 0..4
    unsigned char immediate[4];
    unsigned char immediate_bytes_count;    // 0..4
} encoded_instruction;


void pack_encoded_instruction(encoded_instruction *inst, buffer *buff);
void encoded_instruction_to_str(encoded_instruction *inst, str *s);

