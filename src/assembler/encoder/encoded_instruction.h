#pragma once


// represents encoded bytes, eases encoding & debugging
struct encoded_instruction {
    struct {
        unsigned long have_instruction_prefix : 1;
        unsigned long have_address_size_prefix : 1;
        unsigned long have_operand_size_prefix : 1;
        unsigned long have_segment_override_prefix : 1;
        unsigned long have_opcode_expansion_byte: 1;
        unsigned long have_second_opcode_byte: 1;
        unsigned long have_modregrm: 1;
        unsigned long have_sib: 1;
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
};

void pack_encoded_instruction(struct encoded_instruction *inst, char *buffer, int *buffer_length);
void print_encoded_instruction(struct encoded_instruction *inst, FILE *stream);
