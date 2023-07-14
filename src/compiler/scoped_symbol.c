#include <stdlib.h>
#include "scoped_symbol.h"


scoped_symbol *new_scoped_symbol(mempool *mp, const char *name, ast_data_type *data_type, ast_symbol_type definition, token *token) {
    scoped_symbol *s = mpalloc(mp, scoped_symbol);
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

scoped_symbol *new_scoped_symbol_func_arg(mempool *mp, const char *name, ast_data_type *data_type, int arg_no, token *token) {
    scoped_symbol *s = mpalloc(mp, scoped_symbol);
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

scoped_symbol *new_scoped_symbol_func(mempool *mp, const char *name, ast_function *func, token *token) {
    scoped_symbol *s = mpalloc(mp, scoped_symbol);

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

char *scoped_symbol_type_name(ast_symbol_type st) {
    switch (st) {
        case SYM_VAR: return "var";
        case SYM_FUNC: return "func";
        case SYM_FUNC_ARG: return "arg";
    }
    return "(unknown)";
}
