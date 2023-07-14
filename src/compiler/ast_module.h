#pragma once
#include "../utils/all.h"
#include "ast_declaration.h"


typedef struct ast_module {

    list *statements;  // item type is ast_statement, type of ST_VAR_DECL
    list *functions; // item type is ast_func_declaration

    mempool *mempool;
} ast_module;



ast_module *new_ast_module(mempool *mp);
void ast_module_add_statement(ast_module *m, ast_statement *stmt);
void ast_module_add_function(ast_module *m, ast_func_declaration *func);
void ast_module_print(ast_module *m, FILE *stream);
void ast_module_count_nodes(ast_module *m, int *functions, int *statements, int *expressions);

