#pragma once
#include <stdbool.h>
#include "encoder.h"
#include "asm_instruction.h"
#include "../../utils/buffer.h"
#include "../../linker/reloc_list.h"
#include "../../linker/obj_code.h"
#include "asm_listing.h"
#include "encoded_instruction.h"
#include "encoding_info.h"


enum x86_cpu_mode {
    CPU_MODE_REAL,      // 16 with segments
    CPU_MODE_PROTECTED, // 32
    CPU_MODE_LONG,      // 64
};

struct x86_encoder {
    enum x86_cpu_mode mode;

    buffer *output;
    struct reloc_list *relocations; // symbol relocations to be backfilled

    // old encoder, long switch/if statements
    bool (*encode_old)(struct x86_encoder *encoder, struct asm_instruction_old *instr);
    void (*reset)(struct x86_encoder *encoder);
    void (*free)(struct x86_encoder *encoder);
};

struct x86_encoder *new_x86_encoder(enum x86_cpu_mode mode, buffer *code_out, reloc_list *relocations_out);



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
bool encode_asm_instruction(asm_instruction *inst, struct encoding_info *info, struct encoded_instruction *result);

