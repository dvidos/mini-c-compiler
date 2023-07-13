#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../../run_info.h"
#include "../../err_handler.h"
#include "../lexer/token.h"
#include "../ast_expression.h"
#include "../ast_statement.h"
#include "../ast_symbol.h"
#include "../ast_declaration.h"
#include "../ast_expression.h"
#include "codegen.h"
#include "ir_listing.h"

/*
    For assembly opcodes one can see Intel's books
    https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
    
    Or this useful website:
    http://ref.x86asm.net/coder32.html
*/

static ir_value *_create_ir_value(code_gen *cg, ast_expression *expr);
static void _set_curr_func_name(code_gen *cg, const char *func_name);
static const char *_get_curr_func_name(code_gen *cg);
static int _next_reg_num(code_gen *cg);
static int _next_label_num(code_gen *cg);
static int _curr_loop_num(code_gen *cg);
static void _begin_loop_generation(code_gen *cg);
static void _end_loop_generation(code_gen *cg);

static struct code_gen_ops ops = {
    .generate_for_module = code_gen_generate_for_module,
    .generate_for_function = code_gen_generate_for_function,
    .generate_for_statement = code_gen_generate_for_statement,
    .generate_for_expression = code_gen_generate_for_expression,

    .create_ir_value = _create_ir_value,
    .set_curr_func_name = _set_curr_func_name,
    .get_curr_func_name = _get_curr_func_name,
    .next_reg_num = _next_reg_num,
    .next_label_num = _next_label_num,
    .curr_loop_num = _curr_loop_num,
    .begin_loop_generation = _begin_loop_generation,
    .end_loop_generation = _end_loop_generation,
};

static ir_value *_create_ir_value(code_gen *cg, ast_expression *expr) {
    switch (expr->op) {
        case OP_NUM_LITERAL:
            return new_ir_value_immediate(expr->value.num);

        case OP_CHR_LITERAL:
            return new_ir_value_immediate(expr->value.chr);

        case OP_STR_LITERAL:
            char sym_name[16];
            sprintf(sym_name, "_str%d", cg->ops->next_label_num(cg));
            cg->ir->ops->add(cg->ir, new_ir_data_declaration(strlen(expr->value.str) + 1, expr->value.str, sym_name, IR_GLOBAL_RO));
            return new_ir_value_symbol(sym_name);

        case OP_BOOL_LITERAL:
            return new_ir_value_immediate(expr->value.bln ? 1 : 0);
            break;

        case OP_SYMBOL_NAME:
            return new_ir_value_symbol(expr->value.str);
            break;

        default:
            // otherwise we need to calculate the expression and store it in a register
            ir_value *result = new_ir_value_temp_reg(cg->ops->next_reg_num(cg));
            cg->ops->generate_for_expression(cg, result, expr);
            return result;
    }
}

static void _set_curr_func_name(code_gen *cg, const char *func_name) {
    cg->curr_func_name = func_name;
}

static const char *_get_curr_func_name(code_gen *cg) {
    return cg->curr_func_name;
}

static int _next_reg_num(code_gen *cg) {
    cg->reg_num += 1;
    return cg->reg_num;
}

static int _next_label_num(code_gen *cg) {
    cg->label_num += 1;
    return cg->label_num;
}

static int _curr_loop_num(code_gen *cg) {
    if (cg->loops_stack_len == 0) {
        error("loop number requested by no loops in sight");
        return 0;
    }
    return cg->loops_stack[cg->loops_stack_len - 1];
}

static void _begin_loop_generation(code_gen *cg) {
    if (cg->loops_stack_len >= CODE_GEN_MAX_NESTED_LOOPS) {
        error("cannot generate deeper than %d nested loops", CODE_GEN_MAX_NESTED_LOOPS);
        return;
    }
    cg->label_num += 1;
    cg->loops_stack[cg->loops_stack_len] = cg->label_num;
    cg->loops_stack_len += 1;
}

static void _end_loop_generation(code_gen *cg) {
    if (cg->loops_stack_len == 0) {
        error("finishing a loop that has not started");
        return;
    }
    cg->loops_stack_len -= 1;
}

code_gen *new_code_generator(ir_listing *listing) {
    code_gen *g = malloc(sizeof(code_gen));

    g->curr_func_name = NULL;
    g->ir = listing;
    g->label_num = 0;
    g->reg_num = 0;
    g->loops_stack_len = 0;

    g->ops = &ops;
    return g;
}



