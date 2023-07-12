#pragma once
#include "ir_listing.h"
#include "../ast_declaration.h"
#include "../ast_module.h"


struct code_gen_ops;
#define CODE_GEN_MAX_NESTED_LOOPS 6

typedef struct code_gen {
    ir_listing *ir;
    const char *curr_func_name;
    int reg_num;
    int label_num; // for symbols, labels, ifs etc.
    int loops_stack[CODE_GEN_MAX_NESTED_LOOPS];
    int loops_stack_len;

    struct code_gen_ops *ops;
} code_gen;

code_gen *new_code_generator(ir_listing *listing);

struct code_gen_ops {
    void (*generate_for_module)(code_gen *cg, ast_module *mod);
    void (*generate_for_function)(code_gen *cg, ast_func_declaration *func);
    void (*generate_for_expression)(code_gen *cg, ir_value *lvalue, ast_expression *expr);
    void (*generate_for_statement)(code_gen *cg, ast_statement *stmt);
    
    ir_value *(*create_ir_value)(code_gen *cg, ast_expression *expr);
    void (*set_curr_func_name)(code_gen *cg, const char *func_name);
    const char *(*get_curr_func_name)(code_gen *cg);
    int (*next_reg_num)(code_gen *cg);
    int (*next_label_num)(code_gen *cg);
    int (*curr_loop_num)(code_gen *cg);
    void (*begin_loop_generation)(code_gen *cg);
    void (*end_loop_generation)(code_gen *cg);
};

// codegen.c
void code_gen_generate_for_module(code_gen *cg, ast_module *module);

// codegen_func.c
void code_gen_generate_for_function(code_gen *cg, ast_func_declaration *func);

// codegen_stmt.c
void code_gen_generate_for_statement(code_gen *cg, ast_statement *stmt);

// codegen_expr.c
void code_gen_generate_for_expression(code_gen *cg, ir_value *lvalue, ast_expression *expr);


