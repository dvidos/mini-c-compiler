#include <stddef.h>
#include <stdlib.h>
#include "lexer/token.h"
#include "ast_operator.h"
#include "ast_data_type.h"
#include "ast_statement.h"
#include "ast_declaration.h"

static int count_required_arguments(ast_func_declaration *func);

static struct ast_func_declaration_ops func_ops = {
    .count_required_arguments = count_required_arguments,
};



ast_var_declaration *new_var_declaration(ast_data_type *data_type, const char* var_name, token *token) {
    ast_var_declaration *n = malloc(sizeof(ast_var_declaration));
    n->data_type = data_type;
    n->var_name = var_name;
    n->token = token;
    n->next = NULL;
    return n;
}

ast_func_declaration *new_func_declaration(ast_data_type *return_type, const char* func_name, ast_var_declaration *args_list, ast_statement *body, token *token) {
    ast_func_declaration *n = malloc(sizeof(ast_func_declaration));
    n->func_name = func_name;
    n->return_type = return_type;
    n->args_list = args_list;
    n->stmts_list = body;
    n->token = token;
    n->next = NULL;
    n->ops = &func_ops;
    return n;
}

static int count_required_arguments(ast_func_declaration *func) {
    int count = 0;
    ast_var_declaration *arg = func->args_list;
    while (arg != NULL) {
        count++;
        arg = arg->next;
    }
    return count;
}
