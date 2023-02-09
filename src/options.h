#pragma once
#include <stdbool.h>

struct options {
    bool verbose;
    bool is_32_bits;
    bool is_64_bits;
    char *filename;
    bool elf_test;
};

extern struct options options;

void show_syntax();
void parse_options(int argc, char *argv[]);

