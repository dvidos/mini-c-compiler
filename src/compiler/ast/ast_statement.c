#include <stdlib.h>
#include "ast_variable.h"
#include "ast_function.h"
#include "ast_operator.h"
#include "ast_statement.h"
#include "../lexer/token.h"

static ast_statement *new_any_statement(mempool *mp, ast_statement_type stmt_type, 
        ast_variable *decl, ast_expression *expr, 
        ast_statement *body, ast_statement *else_body,
        token *token
) {
    ast_statement *n = mpalloc(mp, ast_statement);
    n->stmt_type = stmt_type;
    n->decl = decl;
    n->expr = expr;
    n->body = body;
    n->else_body = else_body;

    n->token = token;
    n->next = NULL;
    n->mempool = mp;
    return n;
}

ast_statement *new_ast_statement_block(mempool *mp, ast_statement *stmts_list, token *token) {
    return new_any_statement(mp, ST_BLOCK, NULL, NULL, stmts_list, NULL, token);
}
ast_statement *new_ast_statement_var_decl(mempool *mp, ast_variable *decl, ast_expression *init, token *token) {
    return new_any_statement(mp, ST_VAR_DECL, decl, init, NULL, NULL, token);
}
ast_statement *new_ast_statement_if(mempool *mp, ast_expression *condition, ast_statement *if_body, ast_statement *else_body, token *token) {
    return new_any_statement(mp, ST_IF, NULL, condition, if_body, else_body, token);
}
ast_statement *new_ast_statement_while(mempool *mp, ast_expression *condition, ast_statement *body, token *token) {
    return new_any_statement(mp, ST_WHILE, NULL, condition, body, NULL, token);
}
ast_statement *new_ast_statement_continue(mempool *mp, token *token) {
    return new_any_statement(mp, ST_CONTINUE, NULL, NULL, NULL, NULL, token);
}
ast_statement *new_ast_statement_break(mempool *mp, token *token) {
    return new_any_statement(mp, ST_BREAK, NULL, NULL, NULL, NULL, token);
}
ast_statement *new_ast_statement_return(mempool *mp, ast_expression *return_value, token *token) {
    return new_any_statement(mp, ST_RETURN, NULL, return_value, NULL, NULL, token);
}
ast_statement *new_ast_statement_expression(mempool *mp, ast_expression *expression, token *token) {
    return new_any_statement(mp, ST_EXPRESSION, NULL, expression, NULL, NULL, token);
}


char *ast_statement_type_name(ast_statement_type type) {
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

