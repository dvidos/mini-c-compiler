#pragma once
#include "lexer/token.h"
#include "operators.h"
#include "data_type.h"
#include "expression.h"

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

// ------------------------------------------------------------

// blocks can be nested, so they are a form of statement inside the parent block. 
// they don't appear only in association with an "if" or a "while".
// so, we cannot have blocks that contain statements, but statements of type "block" with a body
// Also, since blocks can have variable declarations, a declaration is either a statement, 
// or a statement (a block) must have a separate list of declarations...

typedef enum statement_type {
    ST_BLOCK,
    ST_VAR_DECL,
    ST_IF,
    ST_WHILE,
    ST_CONTINUE,
    ST_BREAK,
    ST_RETURN,
    ST_EXPRESSION,
} statement_type;

typedef struct ast_statement_node {
    statement_type stmt_type;
    ast_var_decl_node *decl;  // for var declarations, inside functions or blocks
    expr_node *eval; // initial value for declarations, condition for "if" and "while", value for "return"
    ast_statement_node *body; // statements or blocks for if's and while's, a list of statements for blocks
    ast_statement_node *else_body;  // for "else" only

    struct ast_statement_node *next; // for a list of statements in a block
} ast_statement_node;

ast_statement_node *create_ast_block_node(ast_statement_node *body);
ast_statement_node *create_ast_decl_statement(ast_var_decl_node *decl, expr_node *init);
ast_statement_node *create_ast_if_statement(expr_node *condition, ast_statement_node *if_body, ast_statement_node *else_nody);
ast_statement_node *create_ast_while_statement(expr_node *condition, ast_statement_node *body);
ast_statement_node *create_ast_continue_statement();
ast_statement_node *create_ast_break_statement();
ast_statement_node *create_ast_return_statement(expr_node *return_value);
ast_statement_node *create_ast_expr_statement(expr_node *expression);
char *statement_type_name(statement_type type);


