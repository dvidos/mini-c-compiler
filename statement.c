#include <stdlib.h>
#include "ast_node.h"
#include "lexer/token.h"
#include "operators.h"
#include "statement.h"

static ast_statement_node *_create_ast_statement_node(statement_type stmt_type, 
        ast_var_decl_node *decl, expr_node *eval, 
        ast_statement_node *body, ast_statement_node *else_body,
        token *token
) {
    ast_statement_node *n = malloc(sizeof(ast_statement_node));
    n->stmt_type = stmt_type;
    n->decl = decl;
    n->eval = eval;
    n->body = body;
    n->else_body = else_body;

    n->token = token;
    n->next = NULL;
    return n;
}

ast_statement_node *create_ast_block_node(ast_statement_node *stmts_list, token *token) {
    return _create_ast_statement_node(ST_BLOCK, NULL, NULL, stmts_list, NULL, token);
}
ast_statement_node *create_ast_decl_statement(ast_var_decl_node *decl, expr_node *init, token *token) {
    return _create_ast_statement_node(ST_VAR_DECL, decl, init, NULL, NULL, token);
}
ast_statement_node *create_ast_if_statement(expr_node *condition, ast_statement_node *if_body, ast_statement_node *else_body, token *token) {
    return _create_ast_statement_node(ST_IF, NULL, condition, if_body, else_body, token);
}
ast_statement_node *create_ast_while_statement(expr_node *condition, ast_statement_node *body, token *token) {
    return _create_ast_statement_node(ST_WHILE, NULL, condition, body, NULL, token);
}
ast_statement_node *create_ast_continue_statement(token *token) {
    return _create_ast_statement_node(ST_CONTINUE, NULL, NULL, NULL, NULL, token);
}
ast_statement_node *create_ast_break_statement(token *token) {
    return _create_ast_statement_node(ST_BREAK, NULL, NULL, NULL, NULL, token);
}
ast_statement_node *create_ast_return_statement(expr_node *return_value, token *token) {
    return _create_ast_statement_node(ST_RETURN, NULL, return_value, NULL, NULL, token);
}
ast_statement_node *create_ast_expr_statement(expr_node *expression, token *token) {
    return _create_ast_statement_node(ST_EXPRESSION, NULL, expression, NULL, NULL, token);
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

