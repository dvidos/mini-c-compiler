#pragma once
#include "../declaration.h"
#include "../ast.h"


typedef struct code_gen {
    int (*next_reg_num)();
    int (*next_var_num)();
    int (*next_if_num)();
    int (*curr_while_num)();
    void (*push_while)();
    void (*pop_while)();

    void  (*assign_curr_func)(func_declaration *func);
    char *(*curr_func_name)();
    void  (*register_local_var)(var_declaration *decl, bool is_arg, int arg_no);
    int   (*get_local_var_bp_offset)(char *name);

    void (*generate_statement_code)(statement *stmt);
    void (*generate_expression_code)(expression *expr, int target_reg_no, char *target_symbol_name);
} code_gen;


extern code_gen cg;
void generate_module_code(ast_module_node *module);


