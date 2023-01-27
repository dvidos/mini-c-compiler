#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "lexer/token.h"
#include "lexer/lexer.h"
#include "ast_node.h"
#include "ast.h"
#include "operators.h"
#include "scope.h"
#include "symbol.h"
#include "parser/iterator.h"
#include "parser/recursive_descend.h"
#include "parser/shunting_yard.h"
#include "analysis.h"

bool verbose = false;


void read_file(char *filename, char **buffer_pp) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        error(filename, 0, "error opening file");
        return;
    }
    fseek(f, 0, SEEK_END);
    int size = (int)ftell(f);
    fseek(f, 0, SEEK_SET);

    *buffer_pp = malloc(size + 1);
    int bytes_read = (int)fread(*buffer_pp, 1, size, f);
    (*buffer_pp)[bytes_read] = '\0';
    fclose(f);

    if (bytes_read < size) {
        error(filename, 0, "read %d bytes instead of the expected %d", bytes_read, size);
        return;
    }

    printf("Read %d bytes from file %s\n", bytes_read, filename);
    if (verbose) {
        puts("---------------------");
        puts(*buffer_pp);
        puts("---------------------");
    }
}


void parse_file_into_lexer_tokens(char *file_buffer, char *filename) {
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
        add_token(token);
    }

    // one final token, to allow us to always peek at the subsequent token
    add_token(create_token(TOK_EOF, NULL, filename, 999999));

    if (unknown_tokens_exist()) {
        error(filename, 0, "Unknown tokens detected, cannot continue...\n");
        print_tokens("  ", true);
        return;
    }

    printf("Broke file contents into %d tokens\n", count_tokens());
    if (verbose)
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

    if (verbose)
        print_ast();
}

void perform_semantic_analysis() {
    perform_module_analysis(get_ast_root_node());
}

void generate_code() {
    // for now a.out or something simple
}

int main(int argc, char *argv[]) {
    char *file_buffer = NULL;
    char *filename = NULL;
    int err;
    printf("mits mini-c-compiler, v0.01\n");

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'v')
                verbose = true;
        } else {
            filename = argv[i];
        }
    }
    
    if (filename == NULL) {
        printf("Syntax: mcc [-v] file.c\n");
        printf("  -v: verbose\n");
        return 1;
    }

    init_operators();
    init_lexer();
    init_tokens();

    read_file(filename, &file_buffer);
    if (errors_count)
        return 1;
    
    parse_file_into_lexer_tokens(file_buffer, filename);
    if (errors_count)
        return 1;

    // we no longer need this
    free(file_buffer);
    
    parse_abstract_syntax_tree(get_first_token());
    if (errors_count)
        return 1;

    perform_semantic_analysis();
    if (errors_count)
        return 1;
    
    // err = generate_code();
    // if (err)
    //     return 1;

    printf("Success!\n");
    return 0;
}