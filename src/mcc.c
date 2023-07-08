#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err_handler.h"
#include "utils/unit_tests.h"
#include "utils/data_types.h"
#include "utils/data_structs.h"
#include "utils.h"
#include "run_info.h"
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
#include "assembler/ir_to_asm_converter.h"
#include "assembler/assembler.h"
#include "assembler/asm_listing.h"
#include "assembler/encoder/encoder.h"
#include "elf/elf64_contents.h"
#include "linker/linker.h"
#include "linker/obj_code.h"
#include "elf/obj_module.h"
#include "elf/elf_contents.h"


static void perform_end_to_end_test();


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
    
    return unit_tests_outcome(); // prints results and returns success flag
}
#endif 

static void load_source_code(char **source_code) {

    char *p = NULL;
    if (!load_text(run_info->options->filename, &p)) {
        error_at(run_info->options->filename, 0, "Failed loading source code");
        return;
    }
    
    printf("Loaded %ld bytes from file \"%s\"\n", strlen(p), run_info->options->filename);
    if (run_info->options->verbose) {
        printf("------- Source code -------\n");
        printf("%s\n", p);
    }

    (*source_code) = p;
}

static void parse_file_into_lexer_tokens(char *file_buffer, const char *filename, token_list *list) {
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
        error_at(filename, 0, "Unknown tokens detected, cannot continue...\n");
        list->print(list, "  ", true);
        return;
    }

    if (run_info->options->verbose) {
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

    if (run_info->options->verbose) {
        printf("---------- Abstract Syntax Tree ----------\n");
        print_ast(stdout);
    }

    if (run_info->options->generate_ast) {
        char *ast_filename = set_extension(run_info->options->filename, "ast");
        FILE *f = fopen(ast_filename, "w");
        if (f == NULL) {
            error("cannot open file \"%s\" for writing", ast_filename);
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

    if (run_info->options->verbose) {
        printf("--------- Generated Intermediate Representation ---------\n");
        listing->ops->print(listing, stdout);
    }

    // we could run IR optimizations here

    // save result, if required
    if (run_info->options->generate_ir) {
        char *ir_filename = set_extension(run_info->options->filename, "ir");
        FILE *f = fopen(ir_filename, "w");
        if (f == NULL) {
            error("cannot open file \"%s\" for writing", ir_filename);
        }
        listing->ops->print(listing, f);
        fclose(f);
        free(ir_filename);
    }
}

static void process_one_file(mempool *mp, file_run_info *fi) {
    // process one file (load, parse, generate obj module)
    init_lexer();

    char *source_code;
    load_source_code(&source_code);
    if (errors_count)
        return;

    token_list *token_list = new_token_list();
    parse_file_into_lexer_tokens(source_code, str_charptr(fi->source_filename), token_list);
    free(source_code);
    if (errors_count)
        return;

    parse_abstract_syntax_tree(token_list);
    if (errors_count)
        return;

    perform_semantic_analysis();
    if (errors_count)
        return;
    
    ir_listing *ir_listing = new_ir_listing();
    generate_intermediate_code(ir_listing);
    if (errors_count)
        return;

    // we need something that, given ir_listing       generates asm_listing. (.asm file)
    // then an encoder that,   given the asm_listing, generates machine code (.o file)
    // then a linker that,     given the machine code generates the executable (executable file)

    asm_listing *asm_list = new_asm_listing(mp);
    convert_ir_listing_to_asm_listing(mp, ir_listing, asm_list);
    if (errors_count)
        return;

    if (run_info->options->verbose) {
        printf("--------- Generated Assembly Code ---------\n");
        asm_list->ops->print(asm_list, stdout);
    }

    if (run_info->options->generate_asm) {
        char *asm_filename = set_extension(str_charptr(fi->source_filename), "asm");
        FILE *f = fopen(asm_filename, "w");
        if (f == NULL) {
            error("cannot open file \"%s\" for writing", asm_filename);
            return;
        }
        asm_list->ops->print(asm_list, f);
        fclose(f);
        free(asm_filename);
    }

    // ---- old, i386 code ----
    
    char *mod_name = set_extension(str_charptr(fi->source_filename), "o");
    obj_code *cod = new_obj_code();
    cod->vt->set_name(cod, mod_name);
    free(mod_name);

    assemble_listing_into_i386_code(mp, asm_list, cod);
    if (errors_count)
        return;

    if (run_info->options->generate_obj) {
        char *obj_filename = set_extension(str_charptr(fi->source_filename), "obj");
        FILE *f = fopen(obj_filename, "w");
        if (f == NULL) {
            error("cannot open file \"%s\" for writing", obj_filename);
            return;
        }
        if (!cod->vt->save_object_file(cod, f)) {
            error("error writing to file \"%s\"", obj_filename);
            return;
        }
        fclose(f);
        free(obj_filename);
    }

    // ---- new, x86_64 code ----

    // prepare a real module, like real men do.
    obj_module *mod = new_obj_module(mp);
    mod->name = fi->source_filename;
    assemble_listing_into_x86_64_code(mp, asm_list, mod);
    if (errors_count)
        return;
    fi->module = mod;
    
    // save if requested
    if (run_info->options->generate_obj) {
        elf64_contents *elf64 = mod->ops->prepare_elf_contents(mod, ELF_TYPE_REL, mp);
        elf64->ops->save(elf64, str_change_extension(fi->source_filename, "o64"));
    }
}

static void process_all_files(mempool *mp) {
    
    init_operators();

    // for each file, we need to convert into an obj file.
    // then we need to link them all together
    llist *obj_modules = new_llist(mp);

    iterator *run_files_it = llist_create_iterator(run_info->files, mp);
    for_iterator(file_run_info, fi, run_files_it) {
        process_one_file(mp, fi);
        if (errors_count)
            return;
        
        llist_add(obj_modules, fi->module);
    }
    
    // proceeding to link - default runtime files
    llist *obj_files = new_llist(mp);
    llist *lib_files = new_llist(mp);
    llist_add(lib_files, new_str(mp, "libruntime64.a"));

    file_run_info *first_file = llist_get(run_info->files, 0);
    str *executable = str_change_extension(first_file->source_filename, NULL);

    x86_64_link(obj_modules, obj_files, lib_files, 0x400000, executable);
}

static void perform_end_to_end_test() {
    // try to run all the stages, checking at each level.
    // we can start from the end, actually
    // - preprocess -> .i file
    // - compile -> .asm file
    // - assemble -> .obj file
    // - link -> elf file
}

int main(int argc, char *argv[]) {
    printf("mini-c-compiler, v0.01\n");

    mempool *mp = new_mempool();
    initialize_run_info(mp, argc, argv);

    #ifdef INCLUDE_UNIT_TESTS
    if (run_info->options->unit_tests) {
        return run_unit_tests() ? 0 : 1;
    }
    #endif

    if (run_info->options->elf_test) {
        void perform_elf_test();
        perform_elf_test();
        return 0;
    } else if (run_info->options->link_test) {
        void link_test();
        link_test();
        return 0;
    } else if (run_info->options->asm_test) {
        void perform_asm_test();
        perform_asm_test();
        return 0;
    } else if (run_info->options->e2e_test) {
        perform_end_to_end_test();
    }

    if (run_info->options->filename == NULL || llist_is_empty(run_info->files)) {
        show_syntax();
        return 1;
    }

    // process each file, then link them all together
    process_all_files(mp);

    // mempool_print_allocations(mp, stdout);
    mempool_release(mp);

    return errors_count ? 1 : 0;
}

