#include <stddef.h>
#include <stdlib.h>
#include "ast_function.h"

static int count_required_arguments(ast_function *func);

static struct ast_function_ops func_ops = {
    .count_required_arguments = count_required_arguments,
};


ast_function *new_ast_function(mempool *mp, ast_data_type *return_type, const char* func_name, ast_variable *args_list, ast_statement *body, token *token) {
    ast_function *f = mpalloc(mp, ast_function);
    f->func_name = func_name;
    f->return_type = return_type;
    f->args_list = args_list;
    f->stmts_list = body;
    f->token = token;
    f->next = NULL;
    f->ops = &func_ops;
    f->mempool = mp;
    return f;
}

static int count_required_arguments(ast_function *func) {
    int count = 0;
    ast_variable *arg = func->args_list;
    while (arg != NULL) {
        count++;
        arg = arg->next;
    }
    return count;
}
