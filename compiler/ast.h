#pragma once

#include "ast_node.h"

void init_ast();
void ast_add_var(ast_var_decl_node *var);
void ast_add_func(ast_func_decl_node *func);
void print_ast();
