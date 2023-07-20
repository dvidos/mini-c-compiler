#pragma once
#include "../../utils/all.h"
#include "../asm_line.h"


struct encoded_instruction;

// represents encoded bytes, eases encoding & debugging
// many sources, primary: https://gist.github.com/mikesmullin/6259449
typedef struct encoded_instruction {
    struct {
        u8 have_instruction_prefix : 1;
        u8 have_address_size_prefix : 1;
        u8 have_operand_size_prefix : 1;
        u8 have_segment_override_prefix : 1;
        u8 have_rex_prefix : 1;
        u8 have_opcode_expansion_byte: 1;
        u8 have_second_opcode_byte: 1;
        u8 have_modregrm: 1;
        u8 have_sib: 1;
    } flags;
    struct {
        u8 rex_prefix;
        u8 instruction_prefix;
        u8 address_size_prefix;
        u8 operand_size_prefix;
        u8 segment_override_prefix;
        u8 opcode_expansion_byte;
        u8 opcode_byte; // always there
        u8 modregrm_byte;
        u8 sib_byte;
        u8 displacement[4];
        u8 displacement_bytes_count; // 0..4
        u8 immediate[4];
        u8 immediate_bytes_count;    // 0..4
    } values;
    struct encoded_instruction *ops;
    mempool *mempool;
} encoded_instruction;

encoded_instruction *new_encoded_instruction(mempool *mp);

void pack_encoded_instruction(encoded_instruction *inst, bin *buff);
void encoded_instruction_to_str(encoded_instruction *inst, str *s);


struct encoded_instruction_ops {
    void (*pack_encoded_instruction)(encoded_instruction *inst, bin *buff);
    void (*encoded_instruction_to_str)(encoded_instruction *inst, str *s);
    // we could encode more logic, for example turning on REX bits for r8-r15 access.
    // or set the access mode, op subcode etc.
    // must make sure we can mark a relocation for either disp or immediate

    void (*reset)(encoded_instruction *inst);

    // some instructions have expansion byte
    void (*set_instruction_expansion_byte)(encoded_instruction *inst, u8 expansion_byte);

    // some instructions have extension in the reg part of mod-reg-rm
    void (*set_opcode_extension)(encoded_instruction *inst, u8 extension);

    // set ModRM to register
    void (*set_op1_to_reg)(encoded_instruction *inst, gp_register r);

    // set ModRM to memory by symbol
    void (*set_op1_to_mem_by_symbol)(encoded_instruction *inst, str *symbol_name);

    // set ModRM to memory by register +/- displacement
    void (*set_op1_to_mem_by_reg)(encoded_instruction *inst, gp_register reg, s32 displacement);

    // set ModRM to memory by base + [index * size] +/- displacement
    void (*set_op1_to_mem_by_arr)(encoded_instruction *inst, gp_register base, gp_register array_index_reg, u8 array_item_size, s32 displacement);

    // set reg of mod-reg-rm to register
    void (*set_op2_to_reg)(encoded_instruction *inst, gp_register reg);

    // set mod-reg-rm to signify immediate follows
    void (*set_op2_to_imm)(encoded_instruction *inst, gp_register reg);

    // some instructions (e.g. JUMP) take immediate without mod-reg-rm
    void (*set_disp_no_modrm)(encoded_instruction *inst, s32 displacement);

    // set immediate to follow (1, 4, or 8 bytes)
    void (*set_immediate)(encoded_instruction *inst, s32 immediate);
};


