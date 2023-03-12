#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err_handler.h"
#include "utils.h"
#include "options.h"
#include "lexer/token_list.h"
#include "lexer/token.h"
#include "lexer/lexer.h"
#include "declaration.h"
#include "ast.h"
#include "operators.h"
#include "scope.h"
#include "src_symbol.h"
#include "parser/iterator.h"
#include "parser/recursive_descend.h"
#include "parser/shunting_yard.h"
#include "analysis/analysis.h"
#include "codegen/codegen.h"
#include "codegen/ir_listing.h"
#include "binary/binary_gen.h"
#include "elf/elf_contents.h"
#include "elf/elf.h"
#include "x86_encoder/asm_listing.h"
#include "x86_encoder/module.h"



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

void parse_file_into_lexer_tokens(char *file_buffer, char *filename, token_list *list) {
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

void parse_abstract_syntax_tree(token_list *list) {
    init_token_iterator(list);
    init_ast();

    parse_file_using_recursive_descend(list->tokens[0]);
    if (errors_count)
        return;

    // should say "parsed x nodes in AST"
    // int functions;
    // int statements;
    // int expressions;
    // ast_count_nodes(&functions, &statements, &expressions);
    // printf("Parsed tokens into %d functions, %d statements, %d expression nodes\n", functions, statements, expressions);

    if (options.verbose)
        print_ast();
}

void perform_semantic_analysis() {
    perform_module_analysis(get_ast_root_node());
}

void generate_intermediate_code(ir_listing *listing) {
    code_gen *gen = new_code_generator(listing);
    if (errors_count)
        return;
    
    gen->ops->generate_for_module(gen, get_ast_root_node());

    if (options.verbose) {
        printf("--------- Generated Intermediate Representation ---------\n");
        listing->ops->print(listing);
    }

    // we could run IR optimizations here
}

void generate_machine_code(ir_listing *listing) {
    // the last gap to cross.

    // we need something that
    // - given ir_listing 
    // - generates asm_listing. (.asm file)
    // then an encoder,
    // - given the asm_listing,
    // - generate machine code (.o file)
    // then a linker
    // - given the machine code
    // - generate the executable (executable file)

}

int main(int argc, char *argv[]) {
    printf("mini-c-compiler, v0.01\n");

    parse_options(argc, argv);

    if (options.elf_test) {
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
