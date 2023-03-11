#pragma once
#include "ir_listing.h"
#include "../declaration.h"
#include "../ast.h"


typedef struct code_gen {
    int (*next_str_num)();
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
} code_gen;

extern code_gen cg;


struct local_var_info {
    var_declaration *decl;
    bool is_arg;
    int arg_no;
    int bp_offset;
    int size_bytes;
};

struct curr_func_info {
    func_declaration *decl;

    struct local_var_info *vars;
    int vars_count;
    int vars_capacity;
};

extern struct curr_func_info *cg_curr_func;



// codegen.c
ir_listing *generate_module_ir_code(ast_module_node *module);

// codegen_func.c
void generate_function_ir_code(ir_listing *listing, func_declaration *func);

// codegen_stmt.c
void generate_statement_ir_code(ir_listing *listing, statement *stmt);

// codegen_expr.c
typedef struct expr_target expr_target;
expr_target *expr_target_temp_reg(int reg_no);
expr_target *expr_target_return_register();
expr_target *expr_target_stack_location(int frame_offset);
expr_target *expr_target_named_symbol(char *symbol_name);

void generate_expression_ir_code(ir_listing *listing, ir_value *lvalue, expression *expr);


