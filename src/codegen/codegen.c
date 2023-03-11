#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../options.h"
#include "../err_handler.h"
#include "../expression.h"
#include "../lexer/token.h"
#include "../statement.h"
#include "../symbol.h"
#include "../declaration.h"
#include "codegen.h"
#include "ir_listing.h"

/*
    For assembly opcodes one can see Intel's books
    https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
    
    Or this useful website:
    http://ref.x86asm.net/coder32.html
*/

static void _set_curr_func_name(code_gen *cg, char *func_name);
static char *_get_curr_func_name(code_gen *cg);
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
    .set_curr_func_name = _set_curr_func_name,
    .get_curr_func_name = _get_curr_func_name,
    .next_reg_num = _next_reg_num,
    .next_label_num = _next_label_num,
    .curr_loop_num = _curr_loop_num,
    .begin_loop_generation = _begin_loop_generation,
    .end_loop_generation = _end_loop_generation,
};


static void _set_curr_func_name(code_gen *cg, char *func_name) {
    cg->curr_func_name = func_name;
}

static char *_get_curr_func_name(code_gen *cg) {
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
        error(NULL, 0, "loop number requested by no loops in sight");
        return 0;
    }
    return cg->loops_stack[cg->loops_stack_len - 1];
}

static void _begin_loop_generation(code_gen *cg) {
    if (cg->loops_stack_len >= CODE_GEN_MAX_NESTED_LOOPS) {
        error(NULL, 0, "cannot generate deeper than %d nested loops", CODE_GEN_MAX_NESTED_LOOPS);
        return;
    }
    cg->label_num += 1;
    cg->loops_stack[cg->loops_stack_len] = cg->label_num;
    cg->loops_stack_len += 1;
}

static void _end_loop_generation(code_gen *cg) {
    if (cg->loops_stack_len == 0) {
        error(NULL, 0, "finishing a loop that has not started");
        return;
    }
    cg->loops_stack_len -= 1;
}

code_gen *new_code_generator2(ir_listing *listing) {
    code_gen *g = malloc(sizeof(code_gen));

    g->curr_func_name = NULL;
    g->ir = listing;
    g->label_num = 1; // zero is reserved for failure
    g->reg_num = 1;
    g->loops_stack_len = 0;

    g->ops = &ops;
    return g;
}



