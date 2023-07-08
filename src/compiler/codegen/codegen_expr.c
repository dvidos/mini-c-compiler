#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../../run_info.h"
#include "../../err_handler.h"
#include "../expression.h"
#include "../lexer/token.h"
#include "../statement.h"
#include "../src_symbol.h"
#include "../declaration.h"
#include "codegen.h"



static ir_value *resolve_addr(code_gen *cg, expression *expr) {
    // e.g. we can have "a = 1", but also "a[entries[idx].a_offset] = 1"
    
    if (expr->op == OP_SYMBOL_NAME) {
        return new_ir_value_symbol(expr->value.str);

    } else if (expr->op == OP_POINTED_VALUE 
            || expr->op == OP_ARRAY_SUBSCRIPT
            || expr->op == OP_STRUCT_MEMBER_PTR
            || expr->op == OP_STRUCT_MEMBER_REF) {

        ir_value *lvalue = new_ir_value_temp_reg(cg->ops->next_reg_num(cg));
        cg->ops->generate_for_expression(cg, lvalue, expr);
        return lvalue;

    } else {
        error_at(expr->token->filename, expr->token->line_no, 
            "invalid lvalue expression \"%s\", expecting symbol, pointer or array element", 
            oper_debug_name(expr->op)
        );
    }
}

static void gen_func_call(code_gen *cg, ir_value *lvalue, expression *expr) {

    // need to find the func reference?  e.g. "(devices[0]->write)(h, 10, buffer)"
    // remember that C pushes args from right to left (IR opcode PUSH)
    // we can use "scall" IR code for direct symbol call,
    // or         "acall" IR code for calling the address stored in the symbol
    // don't forget to pop things after the call, using the IR code "POP <bytes>"
    // see http://web.archive.org/web/20151010192637/http://www.dound.com/courses/cs143/handouts/17-TAC-Examples.pdf

    // calculate function address
    ir_value *func_addr = resolve_addr(cg, expr->arg1);

    // flatten arguments to be used
    #define MAX_FUNC_ARGS  16
    expression *arg_expressions[MAX_FUNC_ARGS];
    int argc = 0;
    expr->ops->flatten_func_call_args_to_array(expr, arg_expressions, MAX_FUNC_ARGS, &argc);
    if (argc >= MAX_FUNC_ARGS) {
        error_at(expr->token->filename, expr->token->line_no, "only %d function arguments are supported, found %d", MAX_FUNC_ARGS, argc);
        return;
    }

    // prepare the ir_values array
    ir_value **ir_values_arr = malloc(sizeof(ir_value) * argc);
    for (int i = 0; i < argc; i++) {
        ir_values_arr[i] = new_ir_value_temp_reg(cg->ops->next_reg_num(cg));
        cg->ops->generate_for_expression(cg, ir_values_arr[i], arg_expressions[i]);
    }

    cg->ir->ops->add(cg->ir, new_ir_function_call(lvalue, func_addr, argc, ir_values_arr));
}

static void gen_binary_op(code_gen *cg, ir_value *lvalue, expression *expr) {

    ir_value *r1 = new_ir_value_temp_reg(cg->ops->next_reg_num(cg));
    cg->ops->generate_for_expression(cg, r1, expr->arg1);

    ir_value *r2 = new_ir_value_temp_reg(cg->ops->next_reg_num(cg));
    cg->ops->generate_for_expression(cg, r2, expr->arg2);
    
    ir_operation op = IR_NONE;
    switch (expr->op) {
        case OP_ADD: op = IR_ADD; break;
        case OP_SUB: op = IR_SUB; break;
        case OP_MUL: op = IR_MUL; break;
        case OP_DIV: op = IR_DIV; break;
        case OP_MOD: op = IR_MOD; break;
        case OP_BITWISE_AND: op = IR_AND; break;
        case OP_BITWISE_OR: op = IR_OR; break;
        case OP_BITWISE_XOR: op = IR_XOR; break;
        case OP_BITWISE_NOT: op = IR_NOT; break;
        default:
            error_at(expr->token->filename, expr->token->line_no,
                "internal error, not a binary operation (%d, %s)", 
                expr->op, oper_debug_name(expr->op));
            break;
    }

    cg->ir->ops->add(cg->ir, new_ir_three_address_code(lvalue, r1, op, r2));
}

void code_gen_generate_for_expression(code_gen *cg, ir_value *lvalue, expression *expr) {
    switch (expr->op) {
        case OP_NUM_LITERAL:
            cg->ir->ops->add(cg->ir, new_ir_assignment(lvalue, new_ir_value_immediate(expr->value.num)));
            break;

        case OP_CHR_LITERAL:
            cg->ir->ops->add(cg->ir, new_ir_assignment(lvalue, new_ir_value_immediate(expr->value.chr)));
            break;

        case OP_STR_LITERAL:
            char sym_name[16];
            sprintf(sym_name, "_str%d", cg->ops->next_label_num(cg));
            cg->ir->ops->add(cg->ir, new_ir_data_declaration(strlen(expr->value.str) + 1, expr->value.str, sym_name, IR_GLOBAL_RO));
            cg->ir->ops->add(cg->ir, new_ir_assignment(lvalue, new_ir_value_symbol(sym_name)));
            break;

        case OP_BOOL_LITERAL:
            cg->ir->ops->add(cg->ir, new_ir_assignment(lvalue, new_ir_value_immediate(expr->value.bln ? 1 : 0)));
            break;

        case OP_SYMBOL_NAME:
            // maybe the expectation is the contents of the variable and not the address????
            cg->ir->ops->add(cg->ir, new_ir_assignment(lvalue, new_ir_value_symbol(expr->value.str)));
            break;

        case OP_ADD: // fallthrough
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_BITWISE_AND:
        case OP_BITWISE_OR:
        case OP_BITWISE_XOR:
            gen_binary_op(cg, lvalue, expr);
            break;

        case OP_BITWISE_NOT:
            ir_value *rvalue = new_ir_value_temp_reg(cg->ops->next_reg_num(cg));
            cg->ops->generate_for_expression(cg, rvalue, expr->arg2);
            cg->ir->ops->add(cg->ir, new_ir_unary_address_code(lvalue, IR_NOT, rvalue));
            break;

        case OP_FUNC_CALL:
            gen_func_call(cg, lvalue, expr);
            break;

        case OP_ASSIGNMENT:
            // the provided lvalue is ditched (e.g. as in "a = (b = c);")
            ir_value *assignee = resolve_addr(cg, expr->arg1);
            cg->ops->generate_for_expression(cg, assignee, expr->arg2);
            break;

        case OP_PRE_INC: // fallthrough
        case OP_PRE_DEC:
        case OP_POST_INC:
        case OP_POST_DEC:
            bool is_pre = (expr->op == OP_PRE_INC || expr->op == OP_PRE_DEC);
            bool is_inc = (expr->op == OP_PRE_INC || expr->op == OP_POST_INC);
            ir_operation ir_op = is_inc ? IR_ADD : IR_SUB;
            // Pre Inc: result = ++i;
            // r5 = i
            // i = r5 + 1     // first two are same in both cases
            // result = i;    // last assignment depends on pre/post
            // ---------------------------------------
            // Post Inc: result = i++;
            // r5 = i
            // i = r5 + 1     // first two are same in both cases
            // result = r5    // last assignment depends on pre/post
            ir_value *modifiee = resolve_addr(cg, expr->arg1);
            ir_value *temp_reg = new_ir_value_temp_reg(cg->ops->next_reg_num(cg));
            cg->ir->ops->add(cg->ir, new_ir_assignment(temp_reg, modifiee));
            cg->ir->ops->add(cg->ir, new_ir_three_address_code(modifiee, temp_reg, ir_op, new_ir_value_immediate(1)));
            cg->ir->ops->add(cg->ir, new_ir_assignment(lvalue, is_pre ? modifiee : temp_reg));
            break;

        default:
            cg->ir->ops->add(cg->ir, new_ir_comment("(unhandled expresion (%s) follows)", oper_debug_name(expr->op)));
            ir_value *r1 = new_ir_value_temp_reg(cg->ops->next_reg_num(cg));
            ir_value *r2 = new_ir_value_temp_reg(cg->ops->next_reg_num(cg));
            if (expr->arg1) code_gen_generate_for_expression(cg, r1, expr->arg1);
            if (expr->arg2) code_gen_generate_for_expression(cg, r2, expr->arg2);
            break;
    }
}

