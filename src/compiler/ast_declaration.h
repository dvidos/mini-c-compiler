#pragma once
#include "lexer/token.h"
#include "src_operator.h"
#include "ast_data_type.h"
#include "ast_statement.h"

typedef struct ast_statement ast_statement;


typedef struct ast_var_declaration {

    const char *var_name;

    // the declared data type, e.g. "int[]"
    ast_data_type *data_type;

    // house keeping
    token *token;
    struct ast_var_declaration *next; // for function arguments lists
} ast_var_declaration;

ast_var_declaration *new_var_declaration(ast_data_type *data_type, const char* var_name, token *token);



struct ast_func_declaration_ops;

typedef struct ast_func_declaration {

    const char *func_name;

    // used to solve the data type of calling the function
    // and verify returned value types
    ast_data_type *return_type;

    // can be null for functions without arguments
    ast_var_declaration *args_list;

    // the list of contents
    ast_statement *stmts_list;

    // housekeeping
    token *token;
    struct ast_func_declaration *next;
    struct ast_func_declaration_ops *ops;
} ast_func_declaration;

ast_func_declaration *new_func_declaration(ast_data_type *return_type, const char *func_name, ast_var_declaration *args_list, ast_statement *body, token *token);

struct ast_func_declaration_ops {
    int (*count_required_arguments)(ast_func_declaration *func);
};
