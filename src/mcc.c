#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "err_handler.h"
#include "utils/unit_tests.h"
#include "utils/all.h"
#include "utils.h"
#include "run_info.h"
#include "compiler/lexer/token_list.h"
#include "compiler/lexer/token.h"
#include "compiler/lexer/lexer.h"
#include "compiler/ast_declaration.h"
#include "compiler/ast_module.h"
#include "compiler/ast_operator.h"
#include "compiler/scope.h"
#include "compiler/ast_symbol.h"
#include "compiler/parser/token_iterator.h"
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


static bool perform_end_to_end_test();


#ifdef INCLUDE_UNIT_TESTS
static bool run_unit_tests() {

    utils_unit_tests();
    elf_unit_tests();
    
    return unit_tests_outcome(); // prints results and returns success flag
}
#endif 

static str *load_source_code(mempool *mp, str *filename) {

    str *source_code = str_load_file(filename, mp);
    if (source_code == NULL) {
        error_at(str_charptr(filename), 0, "Failed loading source code");
        return NULL;
    }
    
    printf("Loaded %d bytes from file \"%s\"\n", str_len(source_code), str_charptr(filename));
    if (run_info->options->verbose) {
        printf("------- Source code -------\n");
        printf("%s\n", str_charptr(source_code));
    }

    return source_code;
}

static void after_ast_parsed(ast_module *m) {

    if (run_info->options->verbose) {
        printf("---------- Abstract Syntax Tree ----------\n");
        ast_module_print(m, stdout);
    }

    if (run_info->options->generate_ast) {
        char *ast_filename = set_extension(run_info->options->filename, "ast");
        FILE *f = fopen(ast_filename, "w");
        if (f == NULL) {
            error("cannot open file \"%s\" for writing", ast_filename);
        }
        ast_module_print(m, f);
        fclose(f);
        free(ast_filename);
    }
}

static void perform_semantic_analysis(ast_module *m) {
    perform_module_analysis(m);
}

static void generate_intermediate_code(ast_module *ast, ir_listing *listing) {
    code_gen *gen = new_code_generator(listing);
    if (errors_count) return;
    
    gen->ops->generate_for_module(gen, ast);
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

    fi->source_code = load_source_code(mp, fi->source_filename);
    if (fi->source_code == NULL || errors_count)
        return;

    token_list *token_list = new_token_list();
    fi->tokens = lexer_parse_source_code_into_tokens(mp, fi->source_filename, fi->source_code);
    if (fi->tokens == NULL || errors_count)
        return;
    if (!lexer_check_tokens(fi->tokens, fi->source_filename))
        return;

    fi->ast = parse_file_tokens_using_recursive_descend(mp, fi->tokens);
    if (fi->ast == NULL || errors_count)
        return;
    
    after_ast_parsed(fi->ast);
    perform_semantic_analysis(fi->ast);
    if (errors_count)
        return;
    
    ir_listing *ir_listing = new_ir_listing();
    generate_intermediate_code(fi->ast, ir_listing);
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

static bool perform_end_to_end_test() {

    mempool *mp = new_mempool();

    llist *filenames = new_llist_of(mp, 2, new_str(mp, "file1.c"), new_str(mp, "file2.c"));
    llist *sources = new_llist_of(mp, 2, 
        new_str(mp,
            "char *message = \"Hello World\\n\";\n"
            "void greeting();\n"
            "int main() { greeting(); return 0; }\n"),
        new_str(mp, 
            "extern char *message;\n"
            "void greeting() { print(message); }\n")
    );

    llist *token_lists = new_llist(mp);
    for (int i = 0; i < llist_length(filenames); i++) {
        str *filename = llist_get(filenames, i);
        str *source = llist_get(sources, i);
        llist *tokens = lexer_parse_source_code_into_tokens(mp, filename, source);
        if (errors_count || tokens == NULL) return false;
        if (!lexer_check_tokens(tokens, filename)) return false;
        llist_add(token_lists, tokens);
    }

    llist *module_asts = new_llist(mp);
    for_list(token_lists, llist, tokens_list) {
        ast_module *module_ast = parse_file_tokens_using_recursive_descend(mp, tokens_list);
        if (module_ast == NULL || errors_count) return false;
        llist_add(module_asts, module_ast);
        after_ast_parsed(module_ast);
    }

    for_list(module_asts, ast_module, module_ast) {
        // perform_semantic_analysis(mp, module_ast);
        if (errors_count) return false;
    }

    llist *ir_listings = new_llist(mp);
    for_list(module_asts, ast_module, module_ast) {
        ir_listing *ir_lst = NULL; // generate_ir_code(mp, module_ast);
        if (errors_count) return false;
        llist_add(ir_listings, ir_lst);
    }

    llist *asm_listings = new_llist(mp);
    for_list(ir_listings, ir_listing, ir_lst) {
        asm_listing *asm_lst = NULL; // convert_ir_listing_to_asm_listing(mp, ir_lst);
        if (errors_count) return false;
        llist_add(asm_listings, asm_lst);
    }

    asm_listing *l = new_asm_listing(mp);
    l->ops->add_instruction(l, new_asm_instruction(OC_NOP));
    l->ops->add_instruction(l, new_asm_instruction_for_register(OC_PUSH, REG_AX));
    l->ops->add_instruction(l, new_asm_instruction_with_operand(OC_INT, new_asm_operand_imm(0x80)));
    llist_add(asm_listings, l);
    
    llist *obj_modules = new_llist(mp);
    for_list(asm_listings, asm_listing, asm_lst) {
        obj_module *obj = NULL; // assemble_listing_into_x86_64_code(mp, asm_lst);
        if (errors_count) return false;
        llist_add(obj_modules, obj);
    }


    str *executable = new_str(mp, "./end-to-end-test.out");
    bool success = x86_64_link(obj_modules, new_llist(mp), 
            x86_64_std_libraries(mp), x86_64_std_load_address(), executable);
    if (errors_count || !success) return false;

    int err_exit_code = system(str_charptr(executable));
    if (err_exit_code) return false;

    unlink(str_charptr(executable));
    mempool_release(mp);
    return true;
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
        printf("Running end-to-end test... \n");
        bool passed = perform_end_to_end_test();
        printf("%s\n", passed ? "PASSED" : "FAILED");
        return passed ? 0 : 1;
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

