#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "run_info.h"
#include "utils/unit_tests.h"
#include "utils/mempool.h"
#include "utils.h"


// global variable (no be read only)
prog_run_info *run_info;

static prog_run_info the_run_info;
static void parse_options(mempool *mp, int argc, char *argv[]);


void initialize_run_info(mempool *mp, int argc, char *argv[]) {
    run_info = &the_run_info;
    memset(run_info, 0, sizeof(prog_run_info));
    run_info->files = new_llist(mp);
    run_info->options = mempool_alloc(mp, sizeof(prog_run_options), "prog_run_options");
    memset(run_info->options, 0, sizeof(prog_run_options));

    parse_options(mp, argc, argv);
}

void show_syntax() {
    printf("Syntax: mcc [options] file.c\n");
    printf("\t-v           verbose\n");
    // printf("\t-c           compile only\n");
    // printf("\t-S           assemble only\n");
    // printf("\t-E           pre-process only\n");
    printf("\t-m32         generate 32 bits code\n");
    printf("\t-m64         generate 64 bits code\n");
    printf("\t--gen-ast    generate abstract syntax tree file (.ast)\n");
    printf("\t--gen-ir     generate intermediate representation file (.ir)\n");
    printf("\t--gen-asm    generate assembly file (.asm)\n");
    printf("\t--gen-obj    generate object file (.o)\n");
    printf("\t--gen-map    generate linker map file (.map)\n");
    #ifdef INCLUDE_UNIT_TESTS
        printf("\t--unit-tests run unit tests\n");
    #endif
    printf("\t--elf-test   run elf test\n");
    printf("\t--link-test  run link test\n");
    printf("\t--asm-test   run asm test\n");
}

static void parse_options(mempool *mp, int argc, char *argv[]) {

    // defaults
    run_info->options->is_32_bits = true; // the default

    char *p;
    for (int i = 1; i < argc; i++) {
        p = argv[i];

        // first letter not a minus
        if (p[0] != '-') {
            file_run_info *fi = mempool_alloc(mp, sizeof(file_run_info), "file_run_info");
            fi->source_filename = new_str(mp, p);
            llist_add(run_info->files, fi);

            if (run_info->options->filename == NULL)
                run_info->options->filename = p;

            continue;
        }

        // first letter is a minus
        if (strcmp(p, "-v") == 0) {
            run_info->options->verbose = true;
        } else if (strcmp(p, "-m32") == 0) {
            run_info->options->is_32_bits = true;
            run_info->options->is_64_bits = false;
        } else if (strcmp(p, "-m64") == 0) { // what should be the default?
            run_info->options->is_32_bits = false;
            run_info->options->is_64_bits = true;
        } else if (strcmp(p, "--unit-tests") == 0) {
            run_info->options->unit_tests = true;
        } else if (strcmp(p, "--elf-test") == 0) {
            run_info->options->elf_test = true;
        } else if (strcmp(p, "--link-test") == 0) {
            run_info->options->link_test = true;
        } else if (strcmp(p, "--asm-test") == 0) {
            run_info->options->asm_test = true;
        } else if (strcmp(p, "--gen-ast") == 0) {
            run_info->options->generate_ast = true;
        } else if (strcmp(p, "--gen-ir") == 0) {
            run_info->options->generate_ir = true;
        } else if (strcmp(p, "--gen-asm") == 0) {
            run_info->options->generate_asm = true;
        } else if (strcmp(p, "--gen-obj") == 0) {
            run_info->options->generate_obj = true;
        } else if (strcmp(p, "--gen-map") == 0) {
            run_info->options->generate_map = true;
        }
    }

    // derived values that help execution
    run_info->options->pointer_size_bytes = run_info->options->is_32_bits ? 4 : 8;
    run_info->options->register_prefix = run_info->options->is_32_bits ? 'E' : 'R';
}

