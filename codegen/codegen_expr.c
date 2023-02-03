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
#include "../ast_node.h"
#include "interm_repr.h"
#include "codegen.h"


static int next_reg_num();
static void resolve_lvalue(expression *expr, bool *is_calculated, int *lvalue_reg_no);
static void generate_code_for_function_call(expression *expr);
static void generate_code_for_any(expression *expr, int target);

// dragon book, 6.2.1
/*
    An address can be one of the following:
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

static void resolve_lvalue(expression *expr, bool *is_calculated, int *lvalue_reg_no) {
    // e.g. we can have "a = 1", but also "a[offsets[slot].bytes] = 1"
    // ideally we want to get to a Three-Address-Code of
    // essentially we shall end up with a memory location,
    // either by direct symbol name ("a"), or after a calculation.

    // a = 1
    // t1 = offsets
    // t2 = slot_size * slot
    // t1 = t1 + t2
    // t1 = t1 + bytes
    // t3 = a + t1
    // [t3] = 1
    // we need to return info of whether it's a symbol or complex lvalue...

    if (expr->op == OP_SYMBOL_NAME) {
        *is_calculated = false;
        return;
    }

    // so we need to calculate it.
    *is_calculated = true;
    *lvalue_reg_no = next_reg_num();
    generate_code_for_any(expr, *lvalue_reg_no);
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
    flatten_func_call_args_to_array(expr, args, 32, &args_count);

    // in a parallel array, calculate things if needed
    int calculated_regs[32];
    for (int i = 0; i < args_count; i++) {
        if (args[i]->op == OP_SYMBOL_NAME || args[i]->op == OP_NUM_LITERAL) {
            calculated_regs[i] = 0;
        } else {
            int r = next_reg_num();
            generate_code_for_any(args[i], r);
            calculated_regs[i] = r;
        }
    }

    // also, precalculate lvalue if needed
    bool lvalue_calculated;
    int lvalue_reg_no;
    resolve_lvalue(expr->arg1, &lvalue_calculated, &lvalue_reg_no);

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
    if (lvalue_calculated)
        ir_add_str("CALL [t%d]", lvalue_reg_no);
    else
        ir_add_str("CALL %s", expr->arg1->value.str);

    // clean up stack as needed
    if (args_count > 0)
        ir_add_str("POP %d items", args_count);
}


static void generate_code_for_any(expression *expr, int target) {
    int r1, r2;

    // post-order visit, resolve children first, 
    // maybe tell the generator where to store things.

    switch (expr->op) {
        case OP_NUM_LITERAL:
            ir_add_str("t%d = %d", target, expr->value.num);
            break;
        case OP_CHR_LITERAL:
            ir_add_str("t%d = %d", target, (int)expr->value.chr);
            break;
        case OP_STR_LITERAL:
            // we should allocate this in the data segment and use the address?
            ir_add_str("t%d = ADDRESS_OF(\"%s\")", target, expr->value.str);
            break;
        case OP_BOOL_LITERAL:
            ir_add_str("t%d = %d", target, expr->value.bln ? 1 : 0);
            break;
        case OP_SYMBOL_NAME:
            ir_add_str("t%d = %s", target, expr->value.str);
            break;
        case OP_ASSIGNMENT:
            // somehow resolve lvalue?
            ir_add_str("assignment");
            break;
        case OP_ADD:
            r1 = next_reg_num();
            r2 = next_reg_num();
            generate_code_for_any(expr->arg1, r1);
            generate_code_for_any(expr->arg2, r2);
            ir_add_str("t%d = t%d + t%d", target, r1, r2);
            break;
        case OP_SUB:
            r1 = next_reg_num();
            r2 = next_reg_num();
            generate_code_for_any(expr->arg1, r1);
            generate_code_for_any(expr->arg2, r2);
            ir_add_str("t%d = t%d - t%d", target, r1, r2);
            break;
        case OP_MUL:
            r1 = next_reg_num();
            r2 = next_reg_num();
            generate_code_for_any(expr->arg1, r1);
            generate_code_for_any(expr->arg2, r2);
            ir_add_str("t%d = t%d * t%d", target, r1, r2);
            break;
        case OP_DIV:
            r1 = next_reg_num();
            r2 = next_reg_num();
            generate_code_for_any(expr->arg1, r1);
            generate_code_for_any(expr->arg2, r2);
            ir_add_str("t%d = t%d / t%d", target, r1, r2);
            break;
        case OP_BITWISE_AND:
            r1 = next_reg_num();
            r2 = next_reg_num();
            generate_code_for_any(expr->arg1, r1);
            generate_code_for_any(expr->arg2, r2);
            ir_add_str("t%d = t%d AND t%d", target, r1, r2);
            break;
        case OP_BITWISE_OR:
            r1 = next_reg_num();
            r2 = next_reg_num();
            generate_code_for_any(expr->arg1, r1);
            generate_code_for_any(expr->arg2, r2);
            ir_add_str("t%d = t%d OR t%d", target, r1, r2);
            break;
        case OP_BITWISE_XOR:
            r1 = next_reg_num();
            r2 = next_reg_num();
            generate_code_for_any(expr->arg1, r1);
            generate_code_for_any(expr->arg2, r2);
            ir_add_str("t%d = t%d XOR t%d", target, r1, r2);
            break;
        case OP_BITWISE_NOT:
            r1 = next_reg_num();
            r2 = next_reg_num();
            generate_code_for_any(expr->arg2, r1);
            ir_add_str("t%d = 0xFFFFFFFF", r2);
            ir_add_str("t%d = t%d XOR t%d", target, r1, r2); // essentially a NOT
            break;
        case OP_FUNC_CALL:
            generate_code_for_function_call(expr);
            break;

        default:
            if (expr->arg1) generate_code_for_any(expr->arg1, next_reg_num());
            if (expr->arg2) generate_code_for_any(expr->arg2, next_reg_num());
            ir_add_str("expression %s code", oper_debug_name(expr->op));
            break;
    }
}


void generate_expression_code(expression *expr, enum generation_type type) {

    // post-order visit, 
    // each operation is told where to store its result,
    // which is used 

    int destination = 0;
    if (type == GT_RETURNED_VALUE) {
        // put resulting value in r1
        destination = 1;
    } else if (type == GT_BRANCH_DECISION) {
        // we need to resolve one more step for non-comparison expressions?
        destination = 2;
    } else {
        // put resulting value anywhere, we don't care (do we?)
        destination = next_reg_num();
    }
    generate_code_for_any(expr, destination);
}

// ---------------------------------

static int regs_counter = 2;

static int next_reg_num() {
    return ++regs_counter;
}

