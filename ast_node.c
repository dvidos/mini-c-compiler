#include <stdlib.h>
#include "ast_node.h"
#include "lexer/token.h"
#include "operators.h"

// ------------------------------------------------------------------------------

var_declaration *create_var_declaration(data_type *data_type, char* var_name, token *token) {
    var_declaration *n = malloc(sizeof(var_declaration));
    n->data_type = data_type;
    n->var_name = var_name;
    n->token = token;
    n->next = NULL;
    return n;
}

// ------------------------------------------------------------------------------

func_declaration *create_func_declaration(data_type *return_type, char* func_name, var_declaration *args_list, statement *body, token *token) {
    func_declaration *n = malloc(sizeof(func_declaration));
    n->func_name = func_name;
    n->return_type = return_type;
    n->args_list = args_list;
    n->stmts_list = body;
    n->token = token;
    n->next = NULL;
    return n;
}

