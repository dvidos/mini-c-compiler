#pragma once
#include "../ast_node.h"
#include "../expression.h"
#include "../statement.h"
#include "../data_type.h"


void perform_module_analysis(ast_module_node *ast_root);
void perform_declaration_analysis(var_declaration *decl, int arg_no);
void perform_function_analysis(func_declaration *func);

// expr_analysis.c
void perform_expression_analysis(expression *expr);
void verify_expression_type(expression *expr, data_type *needed_type);

// stmt_analysis.c
void perform_statement_analysis(statement *stmt);

