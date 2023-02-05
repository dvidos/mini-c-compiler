#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "options.h"

// global read-only variable
struct options options;


void show_syntax() {
    printf("Syntax: mcc [options] file.c\n");
    printf("\t-v           verbose\n");
    printf("\t-m32         generate 32 bits code\n");
    printf("\t-m64         generate 64 bits code\n");
    printf("\t--elf-test   run elf test\n");
}

void parse_options(int argc, char *argv[]) {

    // defaults
    memset(&options, 0, sizeof(options));
    options.bits = 32;
    options.int_size = 4;

    char *p;
    for (int i = 1; i < argc; i++) {
        p = argv[i];

        // first letter not a minus
        if (p[0] != '-') {
            options.filename = p;
            continue;
        }

        // first letter a minus
        if (strcmp(p, "-v") == 0) {
            options.verbose = true;
        } else if (strcmp(p, "-m32") == 0) {
            options.bits = 32;
            options.int_size = 4;
        } else if (strcmp(p, "-m64") == 0) {
            options.bits = 64;
            options.int_size = 8;
        } else if (strcmp(p, "--elf-test") == 0) {
            options.elf_test = true;
        }
    }
}

