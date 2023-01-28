#pragma once
#include "lexer/token.h"
#include "operators.h"
#include "data_type.h"
#include "expression.h"
#include "statement.h"

// ------------------------------------------------------------------

typedef struct ast_module_node     ast_module_node;     // source-file level
typedef struct ast_func_decl_node  ast_func_decl_node;  // function declaration or definition
typedef struct ast_var_decl_node   ast_var_decl_node;   // type + variable name
typedef struct ast_statement_node  ast_statement_node;  // what can be found in a block

// ------------------------------------------------------------------

typedef struct ast_module_node {

    // list of variables defined at module level
    ast_statement_node *statements_list;

    // list of functions in the source file
    ast_func_decl_node *funcs_list;

} ast_module_node;

// ------------------------------------------------------------

typedef struct ast_var_decl_node {
    data_type *data_type;
    char *var_name;

    token *token;
    struct ast_var_decl_node *next; // for function arguments lists
} ast_var_decl_node;

ast_var_decl_node *create_ast_var_decl_node(data_type *data_type, char* var_name, token *token);

// ------------------------------------------------------------

typedef struct ast_func_decl_node {
    char *func_name;
    data_type *return_type;
    ast_var_decl_node *args_list;
    ast_statement_node *body;

    token *token;
    struct ast_func_decl_node *next; // for function lists
} ast_func_decl_node;

ast_func_decl_node *create_ast_func_decl_node(data_type *return_type, char *func_name, ast_var_decl_node *args_list, ast_statement_node *body, token *token);


