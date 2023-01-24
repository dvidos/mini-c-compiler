#pragma once

#include "ast_node.h"

void init_ast();
void ast_add_statement(ast_statement_node *stmt);
void ast_add_function(ast_func_decl_node *func);
void print_ast();
