#pragma once
#include "../utils/mempool.h"
#include "asm_listing.h"
#include "../linker/obj_code.h"
#include "../elf/obj_module.h"

// given assembly, convert into machine code
void assemble_listing_into_i386_code(mempool *mp, asm_listing *asm_list, obj_code *mod);
void assemble_listing_into_x86_64_code(mempool *mp, asm_listing *asm_list, obj_module *mod);

