#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "options.h"
#include "unit_tests.h"
#include "utils.h"

// global read-only variable
struct options options;


void show_syntax() {
    printf("Syntax: mcc [options] file.c\n");
    printf("\t-v           verbose\n");
    printf("\t-m32         generate 32 bits code\n");
    printf("\t-m64         generate 64 bits code\n");
    printf("\t--gen-ast    generate abstract syntax tree file (.ast)\n");
    printf("\t--gen-ir     generate intermediate representation file (.ir)\n");
    printf("\t--gen-asm    generate assembly file (.asm)\n");
    printf("\t--gen-obj    generate object file (.o)\n");
    printf("\t--gen-map    generate linker map file (.map)\n");
#ifdef INCLUDE_UNIT_TESTS
    printf("\t--unit-tests run unit test\n");
#endif
    printf("\t--elf-test   run elf test\n");
    printf("\t--asm-test   run asm test\n");
}

void parse_options(int argc, char *argv[]) {

    // defaults
    memset(&options, 0, sizeof(options));
    options.is_32_bits = true; // the default

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
            options.is_32_bits = true;
            options.is_64_bits = false;
        } else if (strcmp(p, "-m64") == 0) { // what should be the default?
            options.is_32_bits = false;
            options.is_64_bits = true;
        } else if (strcmp(p, "--unit-tests") == 0) {
            options.unit_tests = true;
        } else if (strcmp(p, "--elf-test") == 0) {
            options.elf_test = true;
        } else if (strcmp(p, "--asm-test") == 0) {
            options.asm_test = true;
        } else if (strcmp(p, "--gen-ast") == 0) {
            options.generate_ast = true;
        } else if (strcmp(p, "--gen-ir") == 0) {
            options.generate_ir = true;
        } else if (strcmp(p, "--gen-asm") == 0) {
            options.generate_asm = true;
        } else if (strcmp(p, "--gen-obj") == 0) {
            options.generate_obj = true;
        } else if (strcmp(p, "--gen-map") == 0) {
            options.generate_map = true;
        }
    }

    // derived values that help execution
    options.pointer_size_bytes = options.is_32_bits ? 4 : 8;
    options.register_prefix = options.is_32_bits ? 'E' : 'R';
}

