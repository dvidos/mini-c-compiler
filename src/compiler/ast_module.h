#pragma once
#include "../utils/data_structs.h"
#include "ast_declaration.h"


typedef struct ast_module {

    // list of variables defined at module level
    ast_statement *statements_list_head;

    // list of functions in the source file
    ast_func_declaration *funcs_list_head;

    mempool *mempool;
} ast_module;



ast_module *new_ast_module(mempool *mp);
void ast_add_statement(ast_module *m, ast_statement *stmt);
void ast_add_function(ast_module *m, ast_func_declaration *func);
void print_ast(ast_module *m, FILE *stream);
void ast_count_nodes(ast_module *m, int *functions, int *statements, int *expressions);

