#pragma once
#include "ast/all.h"


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
    ast_function *func; // if symbol represents a function

    const char *file_name;
    int line_no;
    token *token;
    struct ast_symbol *next;
    mempool *mempool;
} ast_symbol;

ast_symbol *new_ast_symbol(mempool *mp, const char *name, ast_data_type *data_type, ast_symbol_type definition, token *token);
ast_symbol *new_ast_symbol_func_arg(mempool *mp, const char *name, ast_data_type *data_type, int arg_no, token *token);
ast_symbol *new_ast_symbol_func(mempool *mp, const char *name, ast_function *func, token *token);
char *ast_symbol_type_name(ast_symbol_type st);



