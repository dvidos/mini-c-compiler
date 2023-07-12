#pragma once

#include "ast_declaration.h"


typedef struct ast_module {

    // list of variables defined at module level
    ast_statement *statements_list;

    // list of functions in the source file
    ast_func_declaration *funcs_list;

} ast_module;



void init_ast();
ast_module *get_ast_root_node();
void ast_add_statement(ast_statement *stmt);
void ast_add_function(ast_func_declaration *func);
void print_ast(FILE *stream);
void ast_count_nodes(int *functions, int *statements, int *expressions);

