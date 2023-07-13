#include <stdlib.h>
#include "ast_symbol.h"


ast_symbol *new_ast_symbol(const char *name, ast_data_type *data_type, ast_symbol_type definition, const char *file_name, int line_no) {
    ast_symbol *s = malloc(sizeof(ast_symbol));

    s->name = name;
    s->data_type = data_type;
    s->sym_type = definition;
    s->arg_no = -1;
    s->func = NULL;
    s->file_name = file_name;
    s->line_no = line_no;
    s->next = NULL;

    return s;
}

ast_symbol *new_func_arg_symbol(const char *name, ast_data_type *data_type, int arg_no, const char *file_name, int line_no) {
    ast_symbol *s = malloc(sizeof(ast_symbol));

    s->name = name;
    s->data_type = data_type;
    s->sym_type = SYM_FUNC_ARG;
    s->arg_no = arg_no;
    s->func = NULL;
    s->file_name = file_name;
    s->line_no = line_no;
    s->next = NULL;

    return s;
}

ast_symbol *new_func_symbol(const char *name, ast_func_declaration *func, const char *file_name, int line_no) {
    ast_symbol *s = malloc(sizeof(ast_symbol));

    s->name = name;
    s->data_type = func->return_type;
    s->sym_type = SYM_FUNC;
    s->arg_no = -1;
    s->func = func;
    s->file_name = file_name;
    s->line_no = line_no;
    s->next = NULL;

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
