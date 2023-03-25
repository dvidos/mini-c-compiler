#pragma once
#include "ir_listing.h"
#include "../declaration.h"
#include "../ast.h"


struct code_gen_ops;
#define CODE_GEN_MAX_NESTED_LOOPS 6

typedef struct code_gen {
    ir_listing *ir;
    char *curr_func_name;
    int reg_num;
    int label_num; // for symbols, labels, ifs etc.
    int loops_stack[CODE_GEN_MAX_NESTED_LOOPS];
    int loops_stack_len;

    struct code_gen_ops *ops;
} code_gen;

code_gen *new_code_generator(ir_listing *listing);

struct code_gen_ops {
    void (*generate_for_module)(code_gen *cg, ast_module_node *mod);
    void (*generate_for_function)(code_gen *cg, func_declaration *func);
    void (*generate_for_expression)(code_gen *cg, ir_value *lvalue, expression *expr);
    void (*generate_for_statement)(code_gen *cg, statement *stmt);
    
    ir_value *(*create_ir_value)(code_gen *cg, expression *expr);
    void (*set_curr_func_name)(code_gen *cg, char *func_name);
    char *(*get_curr_func_name)(code_gen *cg);
    int (*next_reg_num)(code_gen *cg);
    int (*next_label_num)(code_gen *cg);
    int (*curr_loop_num)(code_gen *cg);
    void (*begin_loop_generation)(code_gen *cg);
    void (*end_loop_generation)(code_gen *cg);
};

// codegen.c
void code_gen_generate_for_module(code_gen *cg, ast_module_node *module);

// codegen_func.c
void code_gen_generate_for_function(code_gen *cg, func_declaration *func);

// codegen_stmt.c
void code_gen_generate_for_statement(code_gen *cg, statement *stmt);

// codegen_expr.c
void code_gen_generate_for_expression(code_gen *cg, ir_value *lvalue, expression *expr);


