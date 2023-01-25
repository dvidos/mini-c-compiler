#include <stdlib.h>
#include "ast_node.h"
#include "lexer/token.h"
#include "operators.h"

// ------------------------------------------------------------------------------

ast_var_decl_node *create_ast_var_decl_node(data_type *data_type, char* var_name) {
    ast_var_decl_node *n = malloc(sizeof(ast_var_decl_node));
    n->node_type = ANT_VAR_DECL;
    n->data_type = data_type;
    n->var_name = var_name;
    n->next = NULL;
    return n;
}

// ------------------------------------------------------------------------------

ast_func_decl_node *create_ast_func_decl_node(data_type *return_type, char* func_name, ast_var_decl_node *args_list, ast_statement_node *body) {
    ast_func_decl_node *n = malloc(sizeof(ast_func_decl_node));
    n->node_type = ANT_FUNC_DECL;
    n->func_name = func_name;
    n->return_type = return_type;
    n->args_list = args_list;
    n->body = body;
    n->next = NULL;
    return n;
}

// ------------------------------------------------------------------------------

static ast_statement_node *_create_ast_statement_node(statement_type stmt_type, 
        ast_var_decl_node *decl, expr_node *eval, 
        ast_statement_node *body, ast_statement_node *else_body
) {
    ast_statement_node *n = malloc(sizeof(ast_statement_node));
    n->node_type = ANT_STATEMENT;
    n->stmt_type = stmt_type;
    n->decl = decl;
    n->eval = eval;
    n->body = body;
    n->else_body = else_body;
    n->next = NULL;
    return n;
}

ast_statement_node *create_ast_block_node(ast_statement_node *stmts_list) {
    return _create_ast_statement_node(ST_BLOCK, NULL, NULL, stmts_list, NULL);
}
ast_statement_node *create_ast_decl_statement(ast_var_decl_node *decl, expr_node *init) {
    return _create_ast_statement_node(ST_VAR_DECL, decl, init, NULL, NULL);
}
ast_statement_node *create_ast_if_statement(expr_node *condition, ast_statement_node *if_body, ast_statement_node *else_body) {
    return _create_ast_statement_node(ST_IF, NULL, condition, if_body, else_body);
}
ast_statement_node *create_ast_while_statement(expr_node *condition, ast_statement_node *body) {
    return _create_ast_statement_node(ST_WHILE, NULL, condition, body, NULL);
}
ast_statement_node *create_ast_continue_statement() {
    return _create_ast_statement_node(ST_CONTINUE, NULL, NULL, NULL, NULL);
}
ast_statement_node *create_ast_break_statement() {
    return _create_ast_statement_node(ST_BREAK, NULL, NULL, NULL, NULL);
}
ast_statement_node *create_ast_return_statement(expr_node *return_value) {
    return _create_ast_statement_node(ST_RETURN, NULL, return_value, NULL, NULL);
}
ast_statement_node *create_ast_expr_statement(expr_node *expression) {
    return _create_ast_statement_node(ST_EXPRESSION, NULL, expression, NULL, NULL);
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

