#include <stdlib.h>
#include "ast_node.h"
#include "lexer/token.h"
#include "operators.h"

// ------------------------------------------------------------------------------

ast_var_decl_node *create_ast_var_decl_node(data_type *data_type, char* var_name, token *token) {
    ast_var_decl_node *n = malloc(sizeof(ast_var_decl_node));
    n->data_type = data_type;
    n->var_name = var_name;
    n->token = token;
    n->next = NULL;
    return n;
}

// ------------------------------------------------------------------------------

ast_func_decl_node *create_ast_func_decl_node(data_type *return_type, char* func_name, ast_var_decl_node *args_list, ast_statement_node *body, token *token) {
    ast_func_decl_node *n = malloc(sizeof(ast_func_decl_node));
    n->func_name = func_name;
    n->return_type = return_type;
    n->args_list = args_list;
    n->body = body;
    n->token = token;
    n->next = NULL;
    return n;
}

