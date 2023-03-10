#pragma once
#include "obj_code.h"
#include "../codegen/ir_listing.h"
#include "asm_listing.h"
#include "encoder.h"


// given an Intemediate Representation listing, generate an assembly listing.
void x86_assemble_ir_listing(ir_listing *ir_list, asm_listing *asm_list);

// given assembly, convert into machine code
void x86_encode_asm_into_machine_code(asm_listing *asm_list, enum x86_cpu_mode mode, obj_code *mod);
