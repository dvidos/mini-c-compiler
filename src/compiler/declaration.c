#include <stddef.h>
#include <stdlib.h>
#include "lexer/token.h"
#include "operators.h"
#include "data_type.h"
#include "statement.h"
#include "declaration.h"

static int count_required_arguments(func_declaration *func);

static struct func_declaration_ops func_ops = {
    .count_required_arguments = count_required_arguments,
};



var_declaration *new_var_declaration(data_type *data_type, const char* var_name, token *token) {
    var_declaration *n = malloc(sizeof(var_declaration));
    n->data_type = data_type;
    n->var_name = var_name;
    n->token = token;
    n->next = NULL;
    return n;
}

func_declaration *new_func_declaration(data_type *return_type, const char* func_name, var_declaration *args_list, statement *body, token *token) {
    func_declaration *n = malloc(sizeof(func_declaration));
    n->func_name = func_name;
    n->return_type = return_type;
    n->args_list = args_list;
    n->stmts_list = body;
    n->token = token;
    n->next = NULL;
    n->ops = &func_ops;
    return n;
}

static int count_required_arguments(func_declaration *func) {
    int count = 0;
    var_declaration *arg = func->args_list;
    while (arg != NULL) {
        count++;
        arg = arg->next;
    }
    return count;
}
