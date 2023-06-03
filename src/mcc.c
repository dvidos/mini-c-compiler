#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err_handler.h"
#include "utils/unit_tests.h"
#include "utils/data_types.h"
#include "utils/data_structs.h"
#include "utils.h"
#include "options.h"
#include "compiler/lexer/token_list.h"
#include "compiler/lexer/token.h"
#include "compiler/lexer/lexer.h"
#include "compiler/declaration.h"
#include "compiler/ast.h"
#include "compiler/operators.h"
#include "compiler/scope.h"
#include "compiler/src_symbol.h"
#include "compiler/parser/iterator.h"
#include "compiler/parser/recursive_descend.h"
#include "compiler/parser/shunting_yard.h"
#include "compiler/analysis/analysis.h"
#include "compiler/codegen/codegen.h"
#include "compiler/codegen/ir_listing.h"
#include "assembler/assembler.h"
#include "assembler/encoder/asm_listing.h"
#include "assembler/encoder/encoder.h"
#include "elf/elf64_contents.h"
#include "linker/linker.h"
#include "linker/obj_code.h"
#include "elf/elf_contents.h"

#ifdef INCLUDE_UNIT_TESTS
static bool run_unit_tests() {

    mempool_unit_tests();
    all_data_types_unit_tests();
    all_data_structs_unit_tests();
    elf_unit_tests();

    void buffer_unit_tests();
    buffer_unit_tests();

    void string_unit_tests();
    string_unit_tests();
    
    void list_unit_tests();
    list_unit_tests();

    return unit_tests_outcome(); // prints results and returns success flag
}
#endif 

static void load_source_code(char **source_code) {

    char *p = NULL;
    if (!load_text(options.filename, &p)) {
        error(options.filename, 0, "Failed loading source code");
        return;
    }
    
    printf("Loaded %ld bytes from file \"%s\"\n", strlen(p), options.filename);
    if (options.verbose) {
        printf("------- Source code -------\n");
        printf("%s\n", p);
    }

    (*source_code) = p;
}

static void parse_file_into_lexer_tokens(char *file_buffer, char *filename, token_list *list) {
    char *p = file_buffer;
    token *token = NULL;
    int err;
    int line_no = 1;

    while (*p != '\0') {
        parse_lexer_token_at_pointer(&p, filename, &line_no, &token);
        if (errors_count)
            return;
        
        if (token == NULL)
            break;
        if (token->type == TOK_COMMENT)
            continue;
        
        list->add(list, token);
    }

    // one final token, to allow us to always peek at the subsequent token
    list->add(list, create_token(TOK_EOF, NULL, filename, 999999));
    if (list->unknown_tokens_exist(list)) {
        error(filename, 0, "Unknown tokens detected, cannot continue...\n");
        list->print(list, "  ", true);
        return;
    }

    if (options.verbose) {
        printf("---- File tokens ----\n");
        list->print(list, "  ", false);
    }
}

static void parse_abstract_syntax_tree(token_list *list) {
    init_token_iterator(list);
    init_ast();

    parse_file_using_recursive_descend(list->tokens[0]);
    if (errors_count)
        return;

    if (options.verbose) {
        printf("---------- Abstract Syntax Tree ----------\n");
        print_ast(stdout);
    }

    if (options.generate_ast) {
        char *ast_filename = set_extension(options.filename, "ast");
        FILE *f = fopen(ast_filename, "w");
        if (f == NULL) {
            error(NULL, 0, "cannot open file \"%s\" for writing", ast_filename);
        }
        print_ast(f);
        fclose(f);
        free(ast_filename);
    }
}

static void perform_semantic_analysis() {
    perform_module_analysis(get_ast_root_node());
}

static void generate_intermediate_code(ir_listing *listing) {
    code_gen *gen = new_code_generator(listing);
    if (errors_count) return;
    
    gen->ops->generate_for_module(gen, get_ast_root_node());
    if (errors_count) return;

    if (options.verbose) {
        printf("--------- Generated Intermediate Representation ---------\n");
        listing->ops->print(listing, stdout);
    }

    // we could run IR optimizations here

    // save result, if required
    if (options.generate_ir) {
        char *ir_filename = set_extension(options.filename, "ir");
        FILE *f = fopen(ir_filename, "w");
        if (f == NULL) {
            error(NULL, 0, "cannot open file \"%s\" for writing", ir_filename);
        }
        listing->ops->print(listing, f);
        fclose(f);
        free(ir_filename);
    }
}

static void generate_machine_code(ir_listing *ir_list) {
    // we need something that, given ir_listing       generates asm_listing. (.asm file)
    // then an encoder that,   given the asm_listing, generates machine code (.o file)
    // then a linker that,     given the machine code generates the executable (executable file)

    asm_listing *asm_list = new_asm_listing();
    x86_assemble_ir_listing(ir_list, asm_list);
    if (errors_count)
        return;

    if (options.verbose) {
        printf("--------- Generated Assembly Code ---------\n");
        asm_list->ops->print(asm_list, stdout);
    }

    if (options.generate_asm) {
        char *asm_filename = set_extension(options.filename, "asm");
        FILE *f = fopen(asm_filename, "w");
        if (f == NULL) {
            error(NULL, 0, "cannot open file \"%s\" for writing", asm_filename);
            return;
        }
        asm_list->ops->print(asm_list, f);
        fclose(f);
        free(asm_filename);
    }

    char *mod_name = set_extension(options.filename, "");
    obj_code *mod = new_obj_code();
    mod->vt->set_name(mod, mod_name);
    free(mod_name);

    x86_encode_asm_into_machine_code(asm_list, CPU_MODE_PROTECTED, mod);
    if (errors_count)
        return;

    if (options.generate_obj) {
        char *obj_filename = set_extension(options.filename, "obj");
        FILE *f = fopen(obj_filename, "w");
        if (f == NULL) {
            error(NULL, 0, "cannot open file \"%s\" for writing", obj_filename);
            return;
        }
        if (!mod->vt->save_object_file(mod, f)) {
            error(NULL, 0, "error writing to file \"%s\"", obj_filename);
            return;
        }
        fclose(f);
        free(obj_filename);
    }

    // link into executable (one or more modules)
    list *modules = new_list();
    modules->v->add(modules, mod);

    char *executable_name = set_extension(options.filename, "");
    x86_link(modules, 0x8048000, executable_name);
    free(executable_name);
    modules->v->free(modules, NULL);
}

int main(int argc, char *argv[]) {
    printf("mini-c-compiler, v0.01\n");

    parse_options(argc, argv);

#ifdef INCLUDE_UNIT_TESTS
    if (options.unit_tests) {
        return run_unit_tests() ? 0 : 1;
    }
#endif

    if (options.elf_test) {
        void perform_elf_test();
        perform_elf_test();
        return 0;
    }

    if (options.asm_test) {
        void perform_asm_test();
        perform_asm_test();
        return 0;
    }

    if (options.filename == NULL) {
        show_syntax();
        return 1;
    }

    init_operators();
    init_lexer();

    char *source_code;
    load_source_code(&source_code);
    if (errors_count)
        return 1;

    token_list *token_list = new_token_list();
    parse_file_into_lexer_tokens(source_code, options.filename, token_list);
    free(source_code);
    if (errors_count)
        return 1;

    parse_abstract_syntax_tree(token_list);
    if (errors_count)
        return 1;

    perform_semantic_analysis();
    if (errors_count)
        return 1;
    
    ir_listing *listing = new_ir_listing();
    generate_intermediate_code(listing);
    if (errors_count)
        return 1;

    generate_machine_code(listing);
    if (errors_count)
        return 1;

    printf("Done!\n");
    return 0;
}
