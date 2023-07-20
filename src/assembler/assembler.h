#pragma once
#include "../utils/mempool.h"
#include "asm_listing.h"
#include "../linker/obj_code.h"
#include "../elf/obj_module.h"


typedef struct assembler assembler;

struct assembler_ops {
    obj_module *(*assemble_listing_into_x86_64_code)(assembler *as, asm_listing *asm_list, str *module_name);
};

typedef struct assembler {
    void *priv_data;
    struct assembler_ops *ops;
} assembler;

assembler *new_assembler(mempool *mp);
void assemble_listing_into_i386_code(mempool *mp, asm_listing *asm_list, obj_code *obj);



#ifdef INCLUDE_UNIT_TESTS
void assembler_unit_tests();
#endif

