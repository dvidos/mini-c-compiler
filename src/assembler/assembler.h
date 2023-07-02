#pragma once
#include "../linker/obj_code.h"
#include "../compiler/codegen/ir_listing.h"
#include "encoder/asm_listing.h"
#include "encoder/encoder.h"
#include "../elf/obj_module.h"


// to be used by asm_allocator
void mkop_for_allocating_stack_space(int size);


// given an Intemediate Representation listing, generate an assembly listing.
void x86_assemble_ir_listing(ir_listing *ir_list, asm_listing *asm_list);

// given assembly, convert into machine code
void x86_encode_asm_into_machine_code(asm_listing *asm_list, enum x86_cpu_mode mode, obj_code *mod);
void encode_asm_into_machine_code_x86_64(mempool *mp, asm_listing *asm_list, obj_module *mod);

