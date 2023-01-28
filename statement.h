#pragma once
#include "lexer/token.h"
#include "operators.h"
#include "data_type.h"
#include "expression.h"
#include "ast_node.h"

// ------------------------------------------------------------------

typedef struct ast_statement_node  ast_statement_node;  // what can be found in a block
typedef struct ast_var_decl_node   ast_var_decl_node;   // type + variable name

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

    token *token; // for line information
    struct ast_statement_node *next; // for a list of statements in a block
} ast_statement_node;

ast_statement_node *create_ast_block_node(ast_statement_node *body, token *token);
ast_statement_node *create_ast_decl_statement(ast_var_decl_node *decl, expr_node *init, token *token);
ast_statement_node *create_ast_if_statement(expr_node *condition, ast_statement_node *if_body, ast_statement_node *else_nody, token *token);
ast_statement_node *create_ast_while_statement(expr_node *condition, ast_statement_node *body, token *token);
ast_statement_node *create_ast_continue_statement(token *token);
ast_statement_node *create_ast_break_statement(token *token);
ast_statement_node *create_ast_return_statement(expr_node *return_value, token *token);
ast_statement_node *create_ast_expr_statement(expr_node *expression, token *token);
char *statement_type_name(statement_type type);


