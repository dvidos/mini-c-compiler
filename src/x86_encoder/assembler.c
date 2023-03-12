#include <stdlib.h>
#include <stddef.h>
#include "../err_handler.h"
#include "module.h"
#include "../codegen/ir_listing.h"
#include "asm_listing.h"


// given an Intemediate Representation listing, generate an assembly listing.
void x86_assemble(
    ir_listing *ir_list,
    asm_listing *asm_list
) {
    error(NULL, 0, "Assembler not implemented yet!");
}
