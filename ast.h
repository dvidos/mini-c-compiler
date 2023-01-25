#pragma once

#include "ast_node.h"

void init_ast();
ast_module_node *get_ast_root_node();
void ast_add_statement(ast_statement_node *stmt);
void ast_add_function(ast_func_decl_node *func);
void print_ast();
void ast_count_nodes(int *functions, int *statements, int *expressions);
