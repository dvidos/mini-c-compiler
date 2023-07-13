#pragma once
#include "../utils/data_structs.h"
#include "ast_declaration.h"


typedef struct ast_module {

    llist *statements;  // item type is ast_statement, type of ST_VAR_DECL
    llist *functions; // item type is ast_func_declaration

    mempool *mempool;
} ast_module;



ast_module *new_ast_module(mempool *mp);
void ast_add_statement(ast_module *m, ast_statement *stmt);
void ast_add_function(ast_module *m, ast_func_declaration *func);
void print_ast(ast_module *m, FILE *stream);
void ast_count_nodes(ast_module *m, int *functions, int *statements, int *expressions);
