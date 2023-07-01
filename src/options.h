#pragma once
#include <stdbool.h>
#include "utils/data_structs.h"


struct options {
    // flags passed in command line
    bool verbose;
    bool is_32_bits;
    bool is_64_bits;

    bool unit_tests;
    bool elf_test;
    bool link_test;
    bool asm_test;

    bool generate_ast;
    bool generate_ir;
    bool generate_asm;
    bool generate_obj;
    bool generate_map;

    char *filename;
    llist *filenames;
    
    // derived values to aid execution
    int pointer_size_bytes;   
    char register_prefix;
};

extern struct options options;

void show_syntax();
void parse_options(mempool *mp, int argc, char *argv[]);

