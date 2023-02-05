#pragma once
#include <stdbool.h>

struct options {
    bool verbose;
    int bits;       // 32 or 64
    int int_size;   // 4 for 32bits, 8 for 64 bits
    char *filename;
    bool elf_test;
};

extern struct options options;

void show_syntax();
void parse_options(int argc, char *argv[]);

