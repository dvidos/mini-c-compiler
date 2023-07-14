#pragma once
#include "../lexer/token.h"
#include "ast_operator.h"
#include "ast_data_type.h"


typedef struct ast_variable {
    const char *var_name;

    // the declared data type, e.g. "int[]"
    ast_data_type *data_type;

    // house keeping
    token *token;
    struct ast_variable *next; // for function arguments lists
    mempool *mempool;
} ast_variable;

ast_variable *new_ast_variable(mempool *mp, ast_data_type *data_type, const char* var_name, token *token);

