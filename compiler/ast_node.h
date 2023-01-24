#pragma once
#include "lexer/token.h"
#include "operators.h"
#include "data_type.h"
#include "expression.h"

// ------------------------------------------------------------------

typedef struct ast_node            ast_node;            // uniform impersonator
typedef struct ast_func_decl_node  ast_func_decl_node;  // function declaration or definition
typedef struct ast_var_decl_node   ast_var_decl_node;   // type + variable name
typedef struct ast_statement_node  ast_statement_node;  // what can be found in a block

// ------------------------------------------------------------------

typedef enum ast_node_type {
    ANT_DATA_TYPE,
    ANT_VAR_DECL,
    ANT_FUNC_DECL,
    ANT_STATEMENT,
    ANT_EXPRESSION,
} ast_node_type;

typedef struct ast_node {
    ast_node_type node_type; // all node types should have this first member
} ast_node;

// ------------------------------------------------------------

typedef struct ast_var_decl_node {
    ast_node_type node_type; // to allow everything to be cast to ast_node

    data_type *data_type;
    char *var_name;
    // maybe an initialization expression could go here

    struct ast_var_decl_node *next; // for function arguments lists
} ast_var_decl_node;

ast_var_decl_node *create_ast_var_decl_node(data_type *data_type, char* var_name);

// ------------------------------------------------------------

typedef struct ast_func_decl_node {
    ast_node_type node_type; // to allow everything to be cast to ast_node

    char *func_name;
    data_type *return_type;
    ast_var_decl_node *args_list;
    ast_statement_node *body;

    struct ast_func_decl_node *next; // for function lists
} ast_func_decl_node;

ast_func_decl_node *create_ast_func_decl_node(data_type *return_type, char *func_name, ast_var_decl_node *args_list, ast_statement_node *body);

// ------------------------------------------------------------

// blocks can be nested, so they are a form of statement inside the parent block. 
// they don't appear only in association with an "if" or a "while".
// so, we cannot have blocks that contain statements, but statements of type "block" with a body
// Also, since blocks can have variable declarations, a declaration is either a statement, 
// or a statement (a block) must have a separate list of declarations...

typedef enum statement_type {
    ST_BLOCK,
    ST_DECLARATION,
    ST_IF,
    ST_WHILE,
    ST_CONTINUE,
    ST_BREAK,
    ST_RETURN,
    ST_EXPRESSION,
} statement_type;

typedef struct ast_statement_node {
    ast_node_type node_type; // to allow everything to be cast to ast_node

    statement_type stmt_type;
    ast_var_decl_node *decl;  // for var declarations, inside functions or blocks
    expr_node *eval; // initial value for declarations, condition for "if" and "while", value for "return"
    ast_statement_node *body; // for blocks, if's and while's
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

// -------------------------------------------------------------

