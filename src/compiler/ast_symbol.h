#pragma once
#include "ast_declaration.h"


typedef enum ast_symbol_type {
    SYM_VAR,
    SYM_FUNC,
    SYM_FUNC_ARG
} ast_symbol_type;

typedef struct ast_symbol {
    const char *name;
    ast_data_type *data_type;
    ast_symbol_type sym_type;
    int arg_no; // zero based argument count, for local variables
    ast_func_declaration *func; // if symbol represents a function

    const char *file_name;
    int line_no;
    struct ast_symbol *next;
} ast_symbol;

ast_symbol *new_ast_symbol(const char *name, ast_data_type *data_type, ast_symbol_type definition, const char *file_name, int line_no);
ast_symbol *new_func_arg_symbol(const char *name, ast_data_type *data_type, int arg_no, const char *file_name, int line_no);
ast_symbol *new_func_symbol(const char *name, ast_func_declaration *func, const char *file_name, int line_no);
char *ast_symbol_type_name(ast_symbol_type st);



