#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err_handler.h"
#include "utils.h"
#include "options.h"
#include "lexer/token.h"
#include "lexer/lexer.h"
#include "declaration.h"
#include "ast.h"
#include "operators.h"
#include "scope.h"
#include "symbol.h"
#include "parser/iterator.h"
#include "parser/recursive_descend.h"
#include "parser/shunting_yard.h"
#include "analysis/analysis.h"
#include "codegen/codegen.h"
#include "codegen/interm_repr.h"
#include "binary/binary_gen.h"
#include "elf/binary_program.h"
#include "elf/elf.h"


void load_source_code(char **source_code) {

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

void parse_file_into_lexer_tokens(char *file_buffer, char *filename, token **first_token) {
    char *p = file_buffer;
    token *token = NULL;
    int err;
    int line_no = 1;

    *first_token = NULL;
    while (*p != '\0') {
        parse_lexer_token_at_pointer(&p, filename, &line_no, &token);
        if (errors_count)
            return;
        
        if (token == NULL)
            break;
        if (token->type == TOK_COMMENT)
            continue;
        add_token(token);
    }

    // one final token, to allow us to always peek at the subsequent token
    add_token(create_token(TOK_EOF, NULL, filename, 999999));
    *first_token = get_first_token();

    if (unknown_tokens_exist()) {
        error(filename, 0, "Unknown tokens detected, cannot continue...\n");
        print_tokens("  ", true);
        return;
    }

    printf("Broke file contents into %d tokens\n", count_tokens());
    if (options.verbose)
        print_tokens("  ", false);
}

void parse_abstract_syntax_tree(token *first) {
    init_token_iterator(first);
    init_ast();

    parse_file_using_recursive_descend(first);
    if (errors_count)
        return;

    // should say "parsed x nodes in AST"
    int functions;
    int statements;
    int expressions;
    ast_count_nodes(&functions, &statements, &expressions);
    printf("Parsed tokens into %d functions, %d statements, %d expression nodes\n",
        functions, statements, expressions);

    if (options.verbose)
        print_ast();
}

void perform_semantic_analysis() {
    perform_module_analysis(get_ast_root_node());
}

void generate_intermediate_code() {
    // for now a.out or something simple
    ir.init();
    generate_module_code(get_ast_root_node());

    if (options.verbose) {
        printf("--------- Generated Intermediate Representation ---------\n");
        ir.dump_symbols();
        ir.dump_data_segment();
        ir.dump_code_segment();
    }
}

void produce_output_files() {
    // get things from Intermediate Representation (ir)
    // generate intel compatible binary output
    // write the elf file

    char *assembly_code;
    // we could write this to file *.asm for fun
    ir.generate_assembly_listing(&assembly_code);
    if (errors_count)
        return;
    
    char *asm_filename = "out.asm";
    if (!save_text("out.asm", assembly_code)) {
        printf("Warning: could not write assembly file %s\n", asm_filename);
    } else {
        printf("Wrote %ld bytes of assembly to file \"%s\"\n", strlen(assembly_code), asm_filename);
        if (options.verbose) {
            printf("----- Assembly code -----\n");
            printf("%s\n", assembly_code);
        }
    }

    binary_program *program;
    generate_binary_code(assembly_code, &program);
    if (errors_count)
        return;

    free(assembly_code);
    
    // "Wrote 1234 bytes to file a.out"
    char *out_filename = "a.out";
    long bytes_written;
    if (!write_elf_file(program, out_filename, &bytes_written)) {
        error(out_filename, 0, "Failed writing file \"%s\"", out_filename);
        return;
    }

    printf("Wrote %ld bytes to final file \"%s\"\n", bytes_written, out_filename);
    if (options.verbose) {
        printf("------- ELF information ---------\n");
        printf("  File type %s\n", 
                (program->flags.is_dynamic_executable) ? "DYN_EXEC" : (
                    program->flags.is_static_executable ? "STAT_EXEC" : (
                        program->flags.is_object_code ? "RELOC_OBJ_CODE" : "(unkowkn)"
                    )
                ));
        printf("  Architecture %d bits\n", 
                    program->flags.is_64_bits ? 64 : 32);
        printf("\n");
        printf("  Segment    Address      Size\n");
        //                0x12345678 123456789
        printf("  .code   0x%08lx %9ld\n", program->code_address, program->code_size);
        printf("  .data   0x%08lx %9ld\n", program->init_data_address, program->init_data_size);
        printf("  .bss    0x%08lx %9ld\n", program->zero_data_address, program->zero_data_size);
        printf("  code entry point at 0x%lx\n", program->code_entry_point);
    }
}

int main(int argc, char *argv[]) {
    printf("mini-c-compiler, v0.01\n");

    parse_options(argc, argv);

    if (options.elf_test) {
        perform_elf_test();
        return 0;
    } else if (options.asm_test) {
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
    init_tokens();

    char *source_code;
    load_source_code(&source_code);
    if (errors_count)
        return 1;

    token *first_token;
    parse_file_into_lexer_tokens(source_code, options.filename, &first_token);
    free(source_code);
    if (errors_count)
        return 1;

    parse_abstract_syntax_tree(first_token);
    if (errors_count)
        return 1;

    perform_semantic_analysis();
    if (errors_count)
        return 1;
    
    generate_intermediate_code();
    if (errors_count)
        return 1;

    // produce_output_files();
    // if (errors_count)
    //     return 1;

    printf("Done!\n");
    return 0;
}
