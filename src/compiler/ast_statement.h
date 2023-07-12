#pragma once
#include "lexer/token.h"
#include "src_operator.h"
#include "ast_data_type.h"
#include "ast_expression.h"
#include "ast_declaration.h"

typedef struct ast_var_declaration ast_var_declaration;

// blocks can be nested, so they are a form of statement inside the parent block. 
// they don't appear only in association with an "if" or a "while".
// so, we cannot have blocks that contain statements, but statements of type "block" with a body
// Also, since blocks can have variable declarations, a declaration is either a statement, 
// or a statement (a block) must have a separate list of declarations...

typedef enum ast_statement_type {
    ST_BLOCK,
    ST_VAR_DECL,
    ST_IF,
    ST_WHILE,
    ST_CONTINUE,
    ST_BREAK,
    ST_RETURN,
    ST_EXPRESSION,
} ast_statement_type;

typedef struct ast_statement {
    // IF, WHILE, VAR_DECL, BLOCK etc
    ast_statement_type stmt_type;

    // variables can be module scope, function scope, or block scope.
    ast_var_declaration *decl;

    // for variables declaration, it's the initial value
    // for "return" it's the return value
    // for "if" and "while" it's the condition
    ast_expression *expr;

    // for a BLOCK, body is a list of statements
    // for IF and WHILE it's the single statement or a block of statements
    // for functions, it's the BLOCK with all the statements
    struct ast_statement *body;
    struct ast_statement *else_body;  // for "else" only

    // house keeping properties
    token *token; // for line information
    struct ast_statement *next; // for a list of statements in a block or function
} ast_statement;

ast_statement *new_statements_block(ast_statement *stmts_list, token *token);
ast_statement *new_var_decl_statement(ast_var_declaration *decl, ast_expression *init, token *token);
ast_statement *new_if_statement(ast_expression *condition, ast_statement *if_body, ast_statement *else_nody, token *token);
ast_statement *new_while_statement(ast_expression *condition, ast_statement *body, token *token);
ast_statement *create_continue_statement(token *token);
ast_statement *new_break_statement(token *token);
ast_statement *new_return_statement(ast_expression *return_value, token *token);
ast_statement *new_expr_statement(ast_expression *expression, token *token);

char *statement_type_name(ast_statement_type type);


