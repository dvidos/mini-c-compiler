#include <stdlib.h>
#include "symbol.h"


symbol *create_symbol(char *name, data_type *data_type, symbol_type definition, char *file_name, int line_no) {
    symbol *s = malloc(sizeof(symbol));

    s->name = name;
    s->data_type = data_type;
    s->sym_type = definition;
    s->arg_no = -1;
    s->file_name = file_name;
    s->line_no = line_no;
    s->next = NULL;

    return s;
}

symbol *create_func_arg_symbol(char *name, data_type *data_type, int arg_no, char *file_name, int line_no) {
    symbol *s = malloc(sizeof(symbol));

    s->name = name;
    s->data_type = data_type;
    s->sym_type = SYM_FUNC_ARG;
    s->arg_no = arg_no;
    s->file_name = file_name;
    s->line_no = line_no;
    s->next = NULL;

    return s;
}
