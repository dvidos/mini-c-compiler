#pragma once
#include "ast_node.h"


void perform_statement_analysis(ast_statement_node *n);
void perform_function_analysis(ast_func_decl_node *n);
