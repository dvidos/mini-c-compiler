#pragma once

#include "ast_node.h"

void init_ast();
ast_module_node *get_ast_root_node();
void ast_add_statement(statement *stmt);
void ast_add_function(func_declaration *func);
void print_ast();
void ast_count_nodes(int *functions, int *statements, int *expressions);
