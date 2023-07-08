#pragma once
#include "../linker/obj_code.h"
#include "../compiler/codegen/ir_listing.h"
#include "encoder/asm_listing.h"
#include "encoder/encoder.h"
#include "../elf/obj_module.h"

// given assembly, convert into machine code
void assemble_listing_into_i386_code(mempool *mp, asm_listing *asm_list, obj_code *mod);
void assemble_listing_into_x86_64_code(mempool *mp, asm_listing *asm_list, obj_module *mod);

