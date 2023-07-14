#pragma once
#include "../lexer/token.h"
#include "ast_operator.h"
#include "ast_data_type.h"
#include "ast_statement.h"

typedef struct ast_statement ast_statement;

struct ast_function_ops;

typedef struct ast_function {

    const char *func_name;

    // used to solve the data type of calling the function
    // and verify returned value types
    ast_data_type *return_type;

    // can be null for functions without arguments
    ast_variable *args_list;

    // the list of contents
    ast_statement *stmts_list;

    // housekeeping
    token *token;
    struct ast_function *next;
    struct ast_function_ops *ops;
    mempool *mempool;
} ast_function;

ast_function *new_ast_function(mempool *mp, ast_data_type *return_type, const char *func_name, ast_variable *args_list, ast_statement *body, token *token);

struct ast_function_ops {
    int (*count_required_arguments)(ast_function *func);
};
