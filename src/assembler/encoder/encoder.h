#pragma once
#include <stdbool.h>
#include "encoder.h"
#include "asm_instruction.h"
#include "../../utils/buffer.h"
#include "../../linker/reloc_list.h"
#include "../../linker/obj_code.h"
#include "asm_listing.h"


enum x86_cpu_mode {
    CPU_MODE_REAL,      // 16 with segments
    CPU_MODE_PROTECTED, // 32
    CPU_MODE_LONG,      // 64
};

struct x86_encoder {
    enum x86_cpu_mode mode;

    buffer *output;
    struct reloc_list *relocations; // symbol relocations to be backfilled

    bool (*encode)(struct x86_encoder *encoder, struct asm_instruction *instr);
    void (*reset)(struct x86_encoder *encoder);
    void (*free)(struct x86_encoder *encoder);
};

struct x86_encoder *new_x86_encoder(enum x86_cpu_mode mode, buffer *code_out, reloc_list *relocations_out);

