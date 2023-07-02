#pragma once
#include <stdbool.h>
#include "encoder.h"
#include "asm_instruction.h"
#include "../../utils/buffer.h"
#include "../../utils/data_structs.h"
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

typedef struct x86_encoder x86_encoder;

struct x86_encoder {
    enum x86_cpu_mode mode;

    buffer *output;
    reloc_list *relocations; // symbol relocations to be backfilled
    mempool *mempool;

    bool (*encode_v4)(x86_encoder *encoder, asm_instruction *instr);
    void (*reset)(x86_encoder *encoder);
    void (*free)(x86_encoder *encoder);
};

x86_encoder *new_x86_encoder(mempool *mp, enum x86_cpu_mode mode, buffer *code_out, reloc_list *relocations_out);


