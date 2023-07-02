#pragma once
#include <stdbool.h>
#include "utils/data_structs.h"
#include "elf/obj_module.h"

typedef struct prog_run_info prog_run_info;
typedef struct file_run_info file_run_info;
typedef struct prog_run_options prog_run_options;

struct prog_run_options {
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
    
    // derived values to aid execution
    int pointer_size_bytes;   
    char register_prefix;
};

struct file_run_info {
    str *source_filename;
    str *source_code;    // the loaded source
    str *assembly_code;  // generated assembly code
    obj_module *module;  // machine code
};

struct prog_run_info {
    llist *files; // item type is file_run_info
    struct prog_run_options *options;
};

// global variables
extern struct prog_run_info *run_info;

void show_syntax();
void initialize_run_info(mempool *mp, int argc, char *argv[]);

