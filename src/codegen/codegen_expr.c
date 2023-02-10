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
#include "interm_repr.h"
#include "codegen.h"


/*
    dragon book, 6.2.1: An address can be one of the following:
    * A name. For convenience, we allow source-program names to appear as
      addresses in three-address code. In an implementation, a source name
      is replaced by a pointer to its symbol-table entry, where all information
      about the name is kept.
    * A constant. In practice, a compiler must deal with many different types
      of constants and variables. Type conversions within expressions are con­
      sidered in Section 6.5.2.
    * A compiler-generated temporary. It is useful, especially in optimizing com­
      pilers, to create a distinct name each time a temporary is needed. These
      temporaries can be combined, if possible, when registers are allocated to
      variables.
*/

static void resolve_lvalue(expression *expr, bool *is_symbol, int *lvalue_reg_no) {
    // e.g. we can have "a = 1", but also "a[offsets[slot].bytes] = 1"
    // ideally we want to get to a Three-Address-Code of
    // essentially we shall end up with a memory location,
    // either by direct symbol name ("a"), or after a calculation.
    // we need to return info of whether it's a symbol or complex lvalue...

    if (expr->op == OP_SYMBOL_NAME) {
        *is_symbol = true;
        return;
    }

    // so we need to calculate it.
    *is_symbol = false;
    *lvalue_reg_no = cg.next_reg_num();
    cg.generate_expression_code(expr, *lvalue_reg_no, NULL);
}

static void generate_code_for_assignment(expression *expr) {

    bool lvalue_is_symbol;
    int lvalue_reg_no;
    resolve_lvalue(expr->arg1, &lvalue_is_symbol, &lvalue_reg_no);

    int rvalue_reg_no;
    if (expr->arg2->op != OP_SYMBOL_NAME && expr->arg2->op != OP_NUM_LITERAL) {
        rvalue_reg_no = cg.next_reg_num();
        cg.generate_expression_code(expr->arg2, rvalue_reg_no, NULL);
    }

    char buffer[10];
    char *lv;
    if (lvalue_is_symbol) {
        lv = expr->arg1->value.str;
    } else {
        sprintf(buffer, "[t%d]", lvalue_reg_no);
        lv = buffer;
    }

    if (expr->arg2->op == OP_SYMBOL_NAME)
        ir_add_str("%s = %s", lv, expr->arg2->value.str);
    else if (expr->arg2->op == OP_NUM_LITERAL)
        ir_add_str("%s = %d", lv, expr->arg2->value.str);
    else 
        ir_add_str("%s = t%d", lv, rvalue_reg_no);
}

static void generate_code_for_function_call(expression *expr) {
    // need to find the func reference?
    // e.g. "(devices[0]->write)(h, 10, buffer)"
    // remember that C pushes args from right to left (IR opcode PUSH)
    // we can use "scall" IR code for direct symbol call,
    // or         "acall" IR code for calling the address stored in the symbol
    // don't forget to pop things after the call, using the IR code "POP <bytes>"
    // see http://web.archive.org/web/20151010192637/http://www.dound.com/courses/cs143/handouts/17-TAC-Examples.pdf

    // now we need to push the results of all the expressions of arguments...
    expression *args[32];
    int args_count = 0;
    expr->ops->flatten_func_call_args_to_array(expr, args, 32, &args_count);

    // in a parallel array, calculate things if needed
    int calculated_regs[32];
    for (int i = 0; i < args_count; i++) {
        if (args[i]->op == OP_SYMBOL_NAME || args[i]->op == OP_NUM_LITERAL) {
            calculated_regs[i] = 0;
        } else {
            int reg = cg.next_reg_num();
            cg.generate_expression_code(args[i], reg, NULL);
            calculated_regs[i] = reg;
        }
    }

    // also, precalculate lvalue if needed
    bool lvalue_is_symbol;
    int lvalue_reg_no;
    resolve_lvalue(expr->arg1, &lvalue_is_symbol, &lvalue_reg_no);

    // push what's needed in reverse order (right-to-left)
    for (int i = args_count - 1; i >= 0; i--) {
        if (args[i]->op == OP_SYMBOL_NAME)
            ir_add_str("PUSH %s", args[i]->value.str);
        else if (args[i]->op == OP_NUM_LITERAL)
            ir_add_str("PUSH %d", args[i]->value.num);
        else
            ir_add_str("PUSH t%d", calculated_regs[i]);
    }

    // call, direct or indirect
    if (lvalue_is_symbol)
        ir_add_str("CALL %s", expr->arg1->value.str);
    else
        ir_add_str("CALL [t%d]", lvalue_reg_no);

    // clean up stack as needed
    if (args_count > 0)
        ir_add_str("POP %d items", args_count);
}

void generate_expression_code(expression *expr, int target_reg, char *target_symbol) {
    int r1, r2;
    char dest_name[32];

    if      (target_reg > 0)        sprintf(dest_name, "t%d", target_reg);
    else if (target_symbol != NULL) sprintf(dest_name, "%s", target_symbol);
    else                            strcpy(dest_name, "nothing");

    switch (expr->op) {
        case OP_NUM_LITERAL:
            ir_add_str("%s = %d", dest_name, expr->value.num);
            break;
        case OP_CHR_LITERAL:
            ir_add_str("%s = %d", dest_name, (int)expr->value.chr);
            break;
        case OP_STR_LITERAL:
            r1 = ir_get_strz_address(expr->value.str, expr->token);
            ir_add_str("%s = .rodata + %d", dest_name, r1);
            break;
        case OP_BOOL_LITERAL:
            ir_add_str("%s = %d", dest_name, expr->value.bln ? 1 : 0);
            break;
        case OP_SYMBOL_NAME:
            ir_add_str("%s = %s", dest_name, expr->value.str);
            break;
        case OP_ADD:
            r1 = cg.next_reg_num();
            r2 = cg.next_reg_num();
            cg.generate_expression_code(expr->arg1, r1, NULL);
            cg.generate_expression_code(expr->arg2, r2, NULL);
            ir_add_str("%s = t%d + t%d", dest_name, r1, r2);
            break;
        case OP_SUB:
            r1 = cg.next_reg_num();
            r2 = cg.next_reg_num();
            cg.generate_expression_code(expr->arg1, r1, NULL);
            cg.generate_expression_code(expr->arg2, r2, NULL);
            ir_add_str("%s = t%d - t%d", dest_name, r1, r2);
            break;
        case OP_MUL:
            r1 = cg.next_reg_num();
            r2 = cg.next_reg_num();
            cg.generate_expression_code(expr->arg1, r1, NULL);
            cg.generate_expression_code(expr->arg2, r2, NULL);
            ir_add_str("%s = t%d * t%d", dest_name, r1, r2);
            break;
        case OP_DIV:
            r1 = cg.next_reg_num();
            r2 = cg.next_reg_num();
            cg.generate_expression_code(expr->arg1, r1, NULL);
            cg.generate_expression_code(expr->arg2, r2, NULL);
            ir_add_str("%s = t%d / t%d", dest_name, r1, r2);
            break;
        case OP_BITWISE_AND:
            r1 = cg.next_reg_num();
            r2 = cg.next_reg_num();
            cg.generate_expression_code(expr->arg1, r1, NULL);
            cg.generate_expression_code(expr->arg2, r2, NULL);
            ir_add_str("%s = t%d AND t%d", dest_name, r1, r2);
            break;
        case OP_BITWISE_OR:
            r1 = cg.next_reg_num();
            r2 = cg.next_reg_num();
            cg.generate_expression_code(expr->arg1, r1, NULL);
            cg.generate_expression_code(expr->arg2, r2, NULL);
            ir_add_str("%s = t%d OR t%d", dest_name, r1, r2);
            break;
        case OP_BITWISE_XOR:
            r1 = cg.next_reg_num();
            r2 = cg.next_reg_num();
            cg.generate_expression_code(expr->arg1, r1, NULL);
            cg.generate_expression_code(expr->arg2, r2, NULL);
            ir_add_str("%s = t%d XOR t%d", dest_name, r1, r2);
            break;
        case OP_BITWISE_NOT:
            r1 = cg.next_reg_num();
            r2 = cg.next_reg_num();
            cg.generate_expression_code(expr->arg2, r1, NULL);
            ir_add_str("%s = 0xFFFFFFFF", r2);
            ir_add_str("%s = t%d XOR t%d", dest_name, r1, r2); // essentially a NOT
            break;
        case OP_ASSIGNMENT:
            generate_code_for_assignment(expr);
            break;
        case OP_FUNC_CALL:
            generate_code_for_function_call(expr);
            break;

        default:
            r1 = cg.next_reg_num();
            r2 = cg.next_reg_num();
            if (expr->arg1) cg.generate_expression_code(expr->arg1, r1, NULL);
            if (expr->arg2) cg.generate_expression_code(expr->arg2, r2, NULL);
            ir_add_str("unknown expression %s code, t%d and t%d", oper_debug_name(expr->op), r1, r2);
            break;
    }
}

