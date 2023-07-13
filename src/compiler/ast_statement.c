#include <stdlib.h>
#include "ast_declaration.h"
#include "lexer/token.h"
#include "ast_operator.h"
#include "ast_statement.h"

static ast_statement *_create_statement(ast_statement_type stmt_type, 
        ast_var_declaration *decl, ast_expression *expr, 
        ast_statement *body, ast_statement *else_body,
        token *token
) {
    ast_statement *n = malloc(sizeof(ast_statement));
    n->stmt_type = stmt_type;
    n->decl = decl;
    n->expr = expr;
    n->body = body;
    n->else_body = else_body;

    n->token = token;
    n->next = NULL;
    return n;
}

ast_statement *new_statements_block(ast_statement *stmts_list, token *token) {
    return _create_statement(ST_BLOCK, NULL, NULL, stmts_list, NULL, token);
}
ast_statement *new_var_decl_statement(ast_var_declaration *decl, ast_expression *init, token *token) {
    return _create_statement(ST_VAR_DECL, decl, init, NULL, NULL, token);
}
ast_statement *new_if_statement(ast_expression *condition, ast_statement *if_body, ast_statement *else_body, token *token) {
    return _create_statement(ST_IF, NULL, condition, if_body, else_body, token);
}
ast_statement *new_while_statement(ast_expression *condition, ast_statement *body, token *token) {
    return _create_statement(ST_WHILE, NULL, condition, body, NULL, token);
}
ast_statement *create_continue_statement(token *token) {
    return _create_statement(ST_CONTINUE, NULL, NULL, NULL, NULL, token);
}
ast_statement *new_break_statement(token *token) {
    return _create_statement(ST_BREAK, NULL, NULL, NULL, NULL, token);
}
ast_statement *new_return_statement(ast_expression *return_value, token *token) {
    return _create_statement(ST_RETURN, NULL, return_value, NULL, NULL, token);
}
ast_statement *new_expr_statement(ast_expression *expression, token *token) {
    return _create_statement(ST_EXPRESSION, NULL, expression, NULL, NULL, token);
}


char *statement_type_name(ast_statement_type type) {
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

