#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atom.h"
#include "defs.h"
#include "token.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"


int read_file(char *file, char **buffer_pp) {
    FILE *f = fopen(file, "r");
    if (f == NULL) {
        printf("error opening file \"%s\"\n", file);
        return ERROR;
    }
    fseek(f, 0, SEEK_END);
    int size = (int)ftell(f);
    fseek(f, 0, SEEK_SET);

    *buffer_pp = malloc(size);
    int bytes_read = (int)fread(*buffer_pp, 1, size, f);
    fclose(f);

    if (bytes_read < size) {
        printf("error reading file %s, read %d instead of %d\n", file, bytes_read, size);
        return ERROR;
    }

    printf("Read %d bytes from file %s\n", bytes_read, file);
    // puts(*buffer_pp);
    return SUCCESS;
}


int parse_file_into_lexer_tokens(char *file_buffer) {
    char *p = file_buffer;
    token *token = NULL;
    int err;

    while (1) {
        err = parse_lexer_token_at_pointer(&p, &token);
        if (err == ERROR)
            return ERROR; 
        if (err == DONE) {
            // one final token, to allow us to always peek
            add_token(create_token(TOK_EOF, NULL));
            break;
        }
        if (token->type == TOK_COMMENT)
            continue;
        
        add_token(token);
    }

    // add one final token, to allow us to always peek ahead
    
    if (unknown_tokens_exist()) {
        printf("Unknown tokens detected, cannot continue...\n");
        print_tokens();
        return ERROR;
    } else {
        printf("Parsed %d tokens\n", count_tokens());
    }

    return SUCCESS;
}

int parse_abstract_syntax_tree(token *first) {
    return ERROR;
    parse_file(first);
    return SUCCESS;
}

int generate_code() {
    // for now a.elf or something
    return ERROR;
}

int main(int argc, char *argv[]) {
    char *file_buffer = NULL;
    int err;
    printf("mits compiler, v0.01\n");

    if (argc < 2) {
        printf("No file name given");
        return 1;
    }

    init_atom();
    init_tokens();

    err = read_file(argv[1], &file_buffer);
    if (err)
        return 1;
    
    err = parse_file_into_lexer_tokens(file_buffer);
    if (err)
        return 1;
    
    err = parse_abstract_syntax_tree(get_first_token());
    if (err)
        return 1;

    // err = generate_code();
    // if (err)
    //     return 1;

    printf("Success!\n");
    return 0;
}
