#pragma once
#include "ast_node.h"


typedef enum symbol_type {
    ST_FUNC,
    ST_VAR,

} symbol_type;

typedef enum synbol_scope {
    SS_GLOBAL,
    SS_FILE,
    SS_PARAMETER,
    SS_LOCAL
} symbol_scope;

typedef struct symbol {
    symbol_type symbol_type;

    char *name;
    data_type *data_type;
    symbol_scope scope; // it really should be a list per scope

    struct symbol *next;
} symbol;

void init_symbols();
symbol *create_symbol(char *name, data_type *data_type);
void add_symbol(symbol *s);
symbol *lookup_symbol(char *name);



