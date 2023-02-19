#pragma once
#include <stdbool.h>

struct options {
    // flags passed in command line
    bool verbose;
    bool is_32_bits;
    bool is_64_bits;
    bool elf_test;
    char *filename;
    
    // derived values to aid execution
    int pointer_size;   
    char register_prefix;
};

extern struct options options;

void show_syntax();
void parse_options(int argc, char *argv[]);

