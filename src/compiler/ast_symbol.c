#include <stdlib.h>
#include "ast_symbol.h"


ast_symbol *new_ast_symbol(mempool *mp, const char *name, ast_data_type *data_type, ast_symbol_type definition, token *token) {
    ast_symbol *s = mpalloc(mp, ast_symbol);
    s->name = name;
    s->data_type = data_type;
    s->sym_type = definition;
    s->arg_no = -1;
    s->func = NULL;
    s->token = token;
    s->next = NULL;
    s->mempool = mp;
    return s;
}

ast_symbol *new_ast_symbol_func_arg(mempool *mp, const char *name, ast_data_type *data_type, int arg_no, token *token) {
    ast_symbol *s = mpalloc(mp, ast_symbol);
    s->name = name;
    s->data_type = data_type;
    s->sym_type = SYM_FUNC_ARG;
    s->arg_no = arg_no;
    s->func = NULL;
    s->token = token;
    s->next = NULL;
    s->mempool = mp;
    return s;
}

ast_symbol *new_ast_symbol_func(mempool *mp, const char *name, ast_function *func, token *token) {
    ast_symbol *s = mpalloc(mp, ast_symbol);

    s->name = name;
    s->data_type = func->return_type;
    s->sym_type = SYM_FUNC;
    s->arg_no = -1;
    s->func = func;
    s->token = token;
    s->next = NULL;
    s->mempool = mp;

    return s;
}

char *ast_symbol_type_name(ast_symbol_type st) {
    switch (st) {
        case SYM_VAR: return "var";
        case SYM_FUNC: return "func";
        case SYM_FUNC_ARG: return "arg";
    }
    return "(unknown)";
}
