#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atom.h"
#include "defs.h"
#include "token.h"
#include "parser.h"

// my working data
struct {
    char *filename;
    char *buffer;
    struct token_list *tokens;
    struct abstract_syntax_tree *ast_root;
} wd;



int read_file(char *file) {
    FILE *f = fopen(file, "r");
    if (f == NULL) {
        printf("error opening file \"%s\"\n", file);
        return ERROR;
    }
    fseek(f, 0, SEEK_END);
    int size = (int)ftell(f);
    fseek(f, 0, SEEK_SET);

    wd.buffer = malloc(size);
    int bytes_read = (int)fread(wd.buffer, 1, size, f);
    fclose(f);

    if (bytes_read < size) {
        printf("error reading file %s, read %d instead of %d\n", file, bytes_read, size);
        return ERROR;
    }

    printf("Read %d bytes from file %s\n", bytes_read, file);
    // puts(wd.buffer);
    return SUCCESS;
}


int parse_file_into_tokens() {
    char *p = wd.buffer;
    token *token = NULL;
    int err;
    while (1) {
        err = parse_token_at_pointer(&p, &token);
        if (err == ERROR)
            return ERROR;
        if (err == DONE)
            break;
        add_token(token);
    }


    if (unknown_tokens_exist()) {
        printf("Unknown tokens detected, cannot continue...\n");
        print_tokens();
        return ERROR;
    } else {
        printf("Parsed %d tokens\n", count_tokens());
    }

    return SUCCESS;
}

int parse_syntax_tree() {
    return SUCCESS;
}

int generate_code() {
    // for now a.elf or something
    return ERROR;
}

int main(int argc, char *argv[]) {
    int err;
    printf("mits compiler, v0.01\n");

    if (argc < 2) {
        printf("No file name given");
        return 1;
    }

    memset(&wd, 0, sizeof(wd));
    init_atom();
    init_tokens();

    err = read_file(argv[1]);
    if (err)
        return 1;
    
    err = parse_file_into_tokens();
    if (err)
        return 1;
    
    // err = parse_syntax_tree();
    // if (err)
    //     return 1;

    // err = generate_code();
    // if (err)
    //     return 1;

    printf("success\n");
    return 0;
}
