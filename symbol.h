#pragma once
#include "ast_node.h"


typedef enum symbol_type {
    SYM_VAR,
    SYM_FUNC,
    SYM_FUNC_ARG
} symbol_type;

typedef struct symbol {
    char *name;
    data_type *data_type;
    symbol_type sym_type;
    int arg_no; // zero based argument count, for local variables
    func_declaration *func; // if symbol represents a function

    char *file_name;
    int line_no;
    struct symbol *next;
} symbol;

symbol *create_symbol(char *name, data_type *data_type, symbol_type definition, char *file_name, int line_no);
symbol *create_func_arg_symbol(char *name, data_type *data_type, int arg_no, char *file_name, int line_no);
symbol *create_func_symbol(char *name, func_declaration *func, char *file_name, int line_no);
char *symbol_type_name(symbol_type st);



