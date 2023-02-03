#include <stdlib.h>
#include "declaration.h"
#include "lexer/token.h"
#include "operators.h"
#include "statement.h"

static statement *_create_statement(statement_type stmt_type, 
        var_declaration *decl, expression *expr, 
        statement *body, statement *else_body,
        token *token
) {
    statement *n = malloc(sizeof(statement));
    n->stmt_type = stmt_type;
    n->decl = decl;
    n->expr = expr;
    n->body = body;
    n->else_body = else_body;

    n->token = token;
    n->next = NULL;
    return n;
}

statement *create_statements_block(statement *stmts_list, token *token) {
    return _create_statement(ST_BLOCK, NULL, NULL, stmts_list, NULL, token);
}
statement *create_var_decl_statement(var_declaration *decl, expression *init, token *token) {
    return _create_statement(ST_VAR_DECL, decl, init, NULL, NULL, token);
}
statement *create_if_statement(expression *condition, statement *if_body, statement *else_body, token *token) {
    return _create_statement(ST_IF, NULL, condition, if_body, else_body, token);
}
statement *create_while_statement(expression *condition, statement *body, token *token) {
    return _create_statement(ST_WHILE, NULL, condition, body, NULL, token);
}
statement *create_continue_statement(token *token) {
    return _create_statement(ST_CONTINUE, NULL, NULL, NULL, NULL, token);
}
statement *create_break_statement(token *token) {
    return _create_statement(ST_BREAK, NULL, NULL, NULL, NULL, token);
}
statement *create_return_statement(expression *return_value, token *token) {
    return _create_statement(ST_RETURN, NULL, return_value, NULL, NULL, token);
}
statement *create_expr_statement(expression *expression, token *token) {
    return _create_statement(ST_EXPRESSION, NULL, expression, NULL, NULL, token);
}


char *statement_type_name(statement_type type) {
    switch (type) {
        case ST_BLOCK: return "block";
        case ST_VAR_DECL: return "declaration";
        case ST_IF: return "if";
        case ST_WHILE: return "while";
        case ST_CONTINUE: return "continue";
        case ST_BREAK: return "break";
        case ST_RETURN: return "return";
        case ST_EXPRESSION: return "expression";
        default: return "*** unnamed ***";
    }
}

