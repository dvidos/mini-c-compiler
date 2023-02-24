#pragma once
#include <stdbool.h>
#include "encoder.h"
#include "instruction.h"
#include "bin_buffer.h"


enum x86_cpu_mode {
    MODE_REAL,      // 16 with segments
    MODE_PROTECTED, // 32
    MODE_LONG,      // 64
};

struct x86_encoder {
    enum x86_cpu_mode mode;
    bool (*encode)(struct x86_encoder *encoder, struct instruction *instr, struct bin_buffer *target);
};

struct x86_encoder *new_x86_encoder(enum x86_cpu_mode mode);
