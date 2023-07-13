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



ast_var_declaration *new_ast_var_declaration(mempool *mp, ast_data_type *data_type, const char* var_name, token *token) {
    ast_var_declaration *v = mpalloc(mp, ast_var_declaration);
    v->data_type = data_type;
    v->var_name = var_name;
    v->token = token;
    v->next = NULL;
    v->mempool = mp;
    return v;
}

ast_func_declaration *new_ast_func_declaration(mempool *mp, ast_data_type *return_type, const char* func_name, ast_var_declaration *args_list, ast_statement *body, token *token) {
    ast_func_declaration *f = mpalloc(mp, ast_func_declaration);
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

static int count_required_arguments(ast_func_declaration *func) {
    int count = 0;
    ast_var_declaration *arg = func->args_list;
    while (arg != NULL) {
        count++;
        arg = arg->next;
    }
    return count;
}
