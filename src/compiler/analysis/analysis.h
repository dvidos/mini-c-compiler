#pragma once
#include "../ast/all.h"
#include "../ast/all.h"
#include "../ast/all.h"
#include "../ast/all.h"
#include "../ast/all.h"


void perform_module_analysis(ast_module *ast_root);
void perform_declaration_analysis(ast_variable *decl, int arg_no);
void perform_function_analysis(ast_function *func);

// expr_analysis.c
void perform_expression_analysis(ast_expression *expr);
void verify_expression_result_type(ast_expression *expr, ast_data_type *needed_type);
void verify_expr_result_integer(ast_expression *expr);
void verify_expr_result_integer_or_pointer(ast_expression *expr);
void verify_expr_result_array_or_pointer(ast_expression *expr);
void verify_expr_result_boolean(ast_expression *expr);
void verify_expr_same_data_types(ast_expression *expr1, ast_expression *expr2, token *token);

// stmt_analysis.c
void perform_statement_analysis(ast_statement *stmt);

