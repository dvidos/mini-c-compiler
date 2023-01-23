#include <stdlib.h>
#include "ast_node.h"
#include "lexer/token.h"
#include "operators.h"

// ------------------------------------------------------------------------------

struct ast_data_type_node *create_ast_data_type_node(token *token, ast_data_type_node *nested) {
    enum type_family family;
    switch (token->type) {
        case TOK_INT_KEYWORD: family = TF_INT; break;
        case TOK_CHAR_KEYWORD: family = TF_CHAR; break;
        case TOK_VOID: family = TF_VOID; break;
        default: family = TF_INT;
    }
    
    ast_data_type_node *n = malloc(sizeof(ast_data_type_node));
    n->node_type = ANT_DATA_TYPE;
    n->family = family;
    n->nested = nested;
    return n;
}

char *data_type_family_name(type_family t) {
    switch (t) {
        case TF_INT: return "int";
        case TF_CHAR: return "char";
        case TF_BOOL: return "bool";
        case TF_VOID: return "void";
        default: return "*** unnamed ***";
    }
}

// ------------------------------------------------------------------------------

ast_var_decl_node *create_ast_var_decl_node(ast_data_type_node *data_type, char* var_name) {
    ast_var_decl_node *n = malloc(sizeof(ast_var_decl_node));
    n->node_type = ANT_VAR_DECL;
    n->data_type = data_type;
    n->var_name = var_name;
    n->next = NULL;
    return n;
}

// ------------------------------------------------------------------------------

ast_func_decl_node *create_ast_func_decl_node(ast_data_type_node *return_type, char* func_name, ast_var_decl_node *args_list, ast_statement_node *body) {
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
        ast_var_decl_node *decl, ast_expression_node *eval, 
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
ast_statement_node *create_ast_decl_statement(ast_var_decl_node *decl, ast_expression_node *init) {
    return _create_ast_statement_node(ST_DECLARATION, decl, init, NULL, NULL);
}
ast_statement_node *create_ast_if_statement(ast_expression_node *condition, ast_statement_node *if_body, ast_statement_node *else_body) {
    return _create_ast_statement_node(ST_IF, NULL, condition, if_body, else_body);
}
ast_statement_node *create_ast_while_statement(ast_expression_node *condition, ast_statement_node *body) {
    return _create_ast_statement_node(ST_WHILE, NULL, condition, body, NULL);
}
ast_statement_node *create_ast_continue_statement() {
    return _create_ast_statement_node(ST_CONTINUE, NULL, NULL, NULL, NULL);
}
ast_statement_node *create_ast_break_statement() {
    return _create_ast_statement_node(ST_BREAK, NULL, NULL, NULL, NULL);
}
ast_statement_node *create_ast_return_statement(ast_expression_node *return_value) {
    return _create_ast_statement_node(ST_RETURN, NULL, return_value, NULL, NULL);
}
ast_statement_node *create_ast_expr_statement(ast_expression_node *expression) {
    return _create_ast_statement_node(ST_EXPRESSION, NULL, expression, NULL, NULL);
}

char *statement_type_name(statement_type type) {
    switch (type) {
        case ST_BLOCK: return "block";
        case ST_DECLARATION: return "declaration";
        case ST_IF: return "if";
        case ST_WHILE: return "while";
        case ST_CONTINUE: return "continue";
        case ST_BREAK: return "break";
        case ST_RETURN: return "return";
        case ST_EXPRESSION: return "expression";
        default: return "*** unnamed ***";
    }
}

// ------------------------------------------------------------------------------

ast_expression_node *create_ast_expression(oper op, ast_expression_node *arg1, ast_expression_node *arg2) {
    ast_expression_node *n = malloc(sizeof(ast_expression_node));
    n->node_type = ANT_EXPRESSION;
    n->op = op;
    n->arg1 = arg1;
    n->arg2 = arg2;
    return n;
}

ast_expression_node *create_ast_expr_name(char *name) {
    ast_expression_node *n = create_ast_expression(OP_SYMBOL_NAME, NULL, NULL);
    n->value.str = name;
    return n;
}
ast_expression_node *create_ast_expr_string_literal(char *str) {
    ast_expression_node *n = create_ast_expression(OP_STR_LITERAL, NULL, NULL);
    n->value.str = str;
    return n;
}
ast_expression_node *create_ast_expr_numeric_literal(char *number) {
    ast_expression_node *n = create_ast_expression(OP_NUM_LITERAL, NULL, NULL);
    int base = 10;
    if (number[0] == '0' && number[1] != '\0') {
        if ((number[1] == 'x' || number[1] == 'X') && number[2] != '\0') {
            base = 16;
            number += 2;
        } else {
            base = 8;
            number += 1;
        }
    }
    n->value.num = strtol(number, NULL, base);
    return n;
}
ast_expression_node *create_ast_expr_char_literal(char chr) {
    ast_expression_node *n = create_ast_expression(OP_CHR_LITERAL, NULL, NULL);
    n->value.chr = chr;
    return n;
}

