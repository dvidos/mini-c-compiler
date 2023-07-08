#pragma once
#include <stdlib.h>
#include "../err_handler.h"
#include "../run_info.h"
#include "codegen/ir_listing.h"
#include "../assembler/asm_listing.h"
#include encoder/asm_allocator.h"

// given an Intemediate Representation listing, generate an assembly listing.
void convert_ir_listing_to_asm_listing(mempool *mp, ir_listing *ir_list, asm_listing *asm_list);
