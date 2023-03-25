#pragma once
#include "../declaration.h"
#include "../expression.h"
#include "../statement.h"
#include "../data_type.h"
#include "../ast.h"


void perform_module_analysis(ast_module_node *ast_root);
void perform_declaration_analysis(var_declaration *decl, int arg_no);
void perform_function_analysis(func_declaration *func);

// expr_analysis.c
void perform_expression_analysis(expression *expr);
void verify_expression_result_type(expression *expr, data_type *needed_type);
void verify_expr_result_integer(expression *expr);
void verify_expr_result_integer_or_pointer(expression *expr);
void verify_expr_result_array_or_pointer(expression *expr);
void verify_expr_result_boolean(expression *expr);
void verify_expr_same_data_types(expression *expr1, expression *expr2, token *token);

// stmt_analysis.c
void perform_statement_analysis(statement *stmt);

