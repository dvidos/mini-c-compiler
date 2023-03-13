#include <stdlib.h>
#include <stddef.h>
#include "../err_handler.h"
#include "../codegen/ir_listing.h"
#include "obj_code.h"
#include "asm_listing.h"
#include "encoder.h"


// given an Intemediate Representation listing, generate an assembly listing.
void x86_assemble_ir_listing(ir_listing *ir_list, asm_listing *asm_list) {
    // also see https://courses.cs.washington.edu/courses/cse401/06sp/codegen.pdf
    error(NULL, 0, "Assembler not implemented yet!");
}

void x86_encode_asm_into_machine_code(asm_listing *asm_list, enum x86_cpu_mode mode, obj_code *mod) {
    // encode this into intel machine code
    struct x86_encoder *enc = new_x86_encoder(mode, mod->text_seg, mod->relocations);
    struct instruction *inst;

    for (int i = 0; i < asm_list->length; i++) {
        inst = &asm_list->instructions[i];

        if (inst->label != NULL) {
            // we don't know if this is exported for now
            mod->symbols->add(mod->symbols, inst->label, mod->text_seg->length, SB_CODE);
        }

        if (!enc->encode(enc, inst)) {
            char str[128];
            instruction_to_string(inst, str, sizeof(str));
            error(NULL, 0, "Failed encoding instruction: '%s'\n", str);
            return;
        }
    }
}


