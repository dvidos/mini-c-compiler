#pragma once

#include "declaration.h"


typedef struct ast_module_node {

    // list of variables defined at module level
    statement *statements_list;

    // list of functions in the source file
    func_declaration *funcs_list;

} ast_module_node;



void init_ast();
ast_module_node *get_ast_root_node();
void ast_add_statement(statement *stmt);
void ast_add_function(func_declaration *func);
void print_ast();
void ast_count_nodes(int *functions, int *statements, int *expressions);

