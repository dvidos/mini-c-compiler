#pragma once
#include "ast_node.h"


typedef enum synbol_scope {
    SS_GLOBAL,
    SS_FILE,
    SS_PARAMETER,
    SS_LOCAL
} symbol_scope;

// where the symbol is defined
typedef enum symbol_definition {
    SD_FILE,
    SD_BLOCK,
    SD_FUNC_ARGUMENT
} symbol_definition;

typedef struct symbol {
    char *name;
    data_type *data_type;
    symbol_definition definition; // where it was defined
    int arg_no; // zero based argument count

    struct symbol *next;
} symbol;

symbol *create_symbol(char *name, data_type *data_type, symbol_definition definition);
symbol *create_func_arg_symbol(char *name, data_type *data_type, int arg_no);




