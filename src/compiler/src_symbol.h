#pragma once
#include "declaration.h"


typedef enum src_symbol_type {
    SYM_VAR,
    SYM_FUNC,
    SYM_FUNC_ARG
} src_symbol_type;

typedef struct src_symbol {
    const char *name;
    data_type *data_type;
    src_symbol_type sym_type;
    int arg_no; // zero based argument count, for local variables
    func_declaration *func; // if symbol represents a function

    const char *file_name;
    int line_no;
    struct src_symbol *next;
} src_symbol;

src_symbol *create_symbol(const char *name, data_type *data_type, src_symbol_type definition, const char *file_name, int line_no);
src_symbol *create_func_arg_symbol(const char *name, data_type *data_type, int arg_no, const char *file_name, int line_no);
src_symbol *create_func_symbol(const char *name, func_declaration *func, const char *file_name, int line_no);
char *symbol_type_name(src_symbol_type st);



