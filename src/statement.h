#pragma once
#include "lexer/token.h"
#include "operators.h"
#include "data_type.h"
#include "expression.h"
#include "declaration.h"

typedef struct var_declaration var_declaration;

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

typedef struct statement {
    // IF, WHILE, VAR_DECL, BLOCK etc
    statement_type stmt_type;

    // variables can be module scope, function scope, or block scope.
    var_declaration *decl;

    // for variables declaration, it's the initial value
    // for "return" it's the return value
    // for "if" and "while" it's the condition
    expression *expr;

    // for a BLOCK, body is a list of statements
    // for IF and WHILE it's the single statement or a block of statements
    // for functions, it's the BLOCK with all the statements
    struct statement *body;
    struct statement *else_body;  // for "else" only

    // house keeping properties
    token *token; // for line information
    struct statement *next; // for a list of statements in a block or function
} statement;

statement *create_statements_block(statement *stmts_list, token *token);
statement *create_var_decl_statement(var_declaration *decl, expression *init, token *token);
statement *create_if_statement(expression *condition, statement *if_body, statement *else_nody, token *token);
statement *create_while_statement(expression *condition, statement *body, token *token);
statement *create_continue_statement(token *token);
statement *create_break_statement(token *token);
statement *create_return_statement(expression *return_value, token *token);
statement *create_expr_statement(expression *expression, token *token);

char *statement_type_name(statement_type type);


