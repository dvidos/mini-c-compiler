#pragma once
#include <stdio.h>
#include <stdbool.h>
#include "asm_instruction.h"

/*
    essentially, since the instruction encoding is so complex
    we need to find a simpler way to approach it.
    all the documentation is useful for disassembling, but does
    not help how to go about choosing how to assemble something.
    
    Displacements could/should be used for structure members.
    SIBs could/should be used for arrays of 1/2/4/8 bytes elements

    But all the above complexity can go away, by code that 
    calculates the exact memory address and puts it in a register.

    For immediate constants, things are even more complex
    Operands have the most significant bit 1,
    Mod+RM have different meanings, direction bit is different, etc.

    Maybe we should start amazingly simple.
*/

struct asm_operation {
    int operation; // ADD, SUB, etc. no sign cognizance
    int operands_size_bits; // 8,16,32,64 (width bit for 8bits, 0x66 prefix for 16bits)
    bool direction_op1_to_op2; // if false, the opposite

    // register or memory
    // goes to the Mod+R/M part of the ModRegRM byte
    struct operand1 { 
        bool is_register;
        bool is_memory_by_reg;
        bool is_memory_by_displacement; // i.e. a symbol
        union {
            enum gp_reg reg;
            struct {
                enum gp_reg pointer_reg;
                long displacement;   // 0 means no displacement
                enum gp_reg array_index_reg;
                int array_item_size; // must be 1,2,4 or 8 for SID to be used
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

void print_asm_operation(struct asm_operation *oper, FILE *stream);




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

void pack_encoded_instruction(struct encoded_instruction *i, char *buffer, int *buffer_length);
void print_encoded_instruction(struct encoded_instruction *i, FILE *stream);




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



bool encode_cpu_operation(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result);

