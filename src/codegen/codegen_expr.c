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


    Running environment in IA-32:
        - access to 4 GB of memory
        Program execution registers:
        - eight general purpose registers (EAX, EBX, ECX, EDX, ESI, EDI, EBP, ESP)
        - six segment registers (16bits) (CS, DS, SS, ES, FS, GS)
        - 32-bit instruction pointer EIP, 32-bit EFLAGS register
        These do integer arithmetic, string manipulation, flow control etc.
        Extra registers: FPU, MMX, XMM, YMM, BND etc for floating, boudaries etc
        I/O ports
        Status registers (CR0 - CR3)
        Memory managmeent registers (GDTR, IDTR, LDTR etc)
        Debug registers (DR0 - DR7)

    Running environment in IA-64:
        - access to 17179869183 GB of memory...
        Program execution registers:
        - eight general purpose registers (RAX, RBX, RCX, RDX, RSI, RDI, RBP, RSP)
        - six segment registers (CS, DS, SS, ES, FS, GS)
        - 64-bit instruction pointer RIP, 64-bit RFLAGS register
        These do integer arithmetic, string manipulation, flow control etc.
        Extra registers: FPU, MMX, XMM, YMM, BND etc for floating, boudaries etc
        I/O ports
        Status registers (CR0 - CR3)
        Memory managmeent registers (GDTR, IDTR, LDTR etc)
        Debug registers (DR0 - DR7)

    


    
*/





static expr_target *resolve_lvalue_target(expression *expr) {
    // e.g. we can have "a = 1", but also "a[offsets[slot].bytes] = 1"
    // ideally we want to get to a Three-Address-Code of
    // essentially we shall end up with a memory location,
    // either by direct symbol name ("a"), or after a calculation.

    if (expr->op == OP_SYMBOL_NAME) {
        return expr_target_named_symbol(expr->value.str);

    } else if (expr->op == OP_POINTED_VALUE 
            || expr->op == OP_ARRAY_SUBSCRIPT
            || expr->op == OP_STRUCT_MEMBER_PTR
            || expr->op == OP_STRUCT_MEMBER_REF) {
        // generate expression, store result in temp reg
        expr_target *address_target = expr_target_temp_reg(cg.next_reg_num());
        generate_expression_code(expr, address_target);

        // use result as pointer
        return expr_target_pointed_by_temp_reg(address_target->u.value);

    } else {
        error(expr->token->filename, expr->token->line_no, 
            "invalid lvalue expression \"%s\", expecting symbol, pointer or array element", 
            oper_debug_name(expr->op)
        );
    }
}

static void generate_code_for_function_call(expression *expr) {
    // need to find the func reference?  e.g. "(devices[0]->write)(h, 10, buffer)"
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
            expr_target *target = expr_target_temp_reg(cg.next_reg_num());
            generate_expression_code(args[i], target);
            calculated_regs[i] = target->u.value;
        }
    }

    // also, precalculate lvalue if needed
    bool lvalue_is_symbol;
    int lvalue_reg_no;
    expr_target *target = resolve_lvalue_target(expr->arg1);

    // push what's needed in reverse order (right-to-left)
    for (int i = args_count - 1; i >= 0; i--) {
        if (args[i]->op == OP_SYMBOL_NAME)
            ir.add_str("PUSH %s", args[i]->value.str);
        else if (args[i]->op == OP_NUM_LITERAL)
            ir.add_str("PUSH %d", args[i]->value.num);
        else
            ir.add_str("PUSH t%d", calculated_regs[i]);
    }

    // call, direct or indirect
    if (lvalue_is_symbol)
        ir.add_str("CALL %s", expr->arg1->value.str);
    else
        ir.add_str("CALL [t%d]", lvalue_reg_no);

    // clean up stack as needed
    if (args_count > 0)
        ir.add_str("POP %d items", args_count);
}

static void generate_code_for_binary_operation(expression *expr, expr_target *target) {
    expr_target *t1;
    expr_target *t2;

    t1 = expr_target_temp_reg(cg.next_reg_num());
    t2 = expr_target_temp_reg(cg.next_reg_num());

    generate_expression_code(expr->arg1, t1);
    generate_expression_code(expr->arg2, t2);
    
    char o = ' ';
    switch (expr->op) {
        case OP_ADD: o = '+'; break;
        case OP_SUB: o = '+'; break;
        case OP_MUL: o = '*'; break;
        case OP_DIV: o = '/'; break;
        case OP_BITWISE_AND: o = '&'; break;
        case OP_BITWISE_OR: o = '|'; break;
        case OP_BITWISE_XOR: o = '^'; break;
        case OP_BITWISE_NOT: o = '~'; break;

        default:
            error(expr->token->filename, expr->token->line_no,
                "internal error, not a binary operation (%d, %s)", 
                expr->op, oper_debug_name(expr->op));
            break;
    }

    // we have operation in o
    ir.add_tac(target, "t%d %c t%d", t1->u.value, o, t2->u.value);
    // ir.add_str("%s = t%d + t%d", dest_name, t1->u.value, t2->u.value);
}

void generate_expression_code(expression *expr, expr_target *target) {
    expr_target *t1;
    expr_target *t2;

    switch (expr->op) {
        case OP_NUM_LITERAL:
            ir.add_tac(target, " = %d", expr->value.num);
            break;
        case OP_CHR_LITERAL:
            ir.add_tac(target, " = %d", (int)expr->value.chr);
            break;
        case OP_STR_LITERAL:
            int offset = ir.reserve_strz(expr->value.str);
            ir.add_tac(target, " = .data + %d", offset);
            break;
        case OP_BOOL_LITERAL:
            ir.add_tac(target, " = %d", expr->value.bln ? 1 : 0);
            break;
        case OP_SYMBOL_NAME:
            ir.add_tac(target, " = %s", expr->value.str);
            break;

        case OP_ADD: // fallthrough
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_BITWISE_AND:
        case OP_BITWISE_OR:
        case OP_BITWISE_XOR:
            generate_code_for_binary_operation(expr, target);
            break;

        case OP_BITWISE_NOT:
            expr_target *t1 = expr_target_temp_reg(cg.next_reg_num());
            generate_expression_code(expr->arg2, t1);
            ir.add_tac(target, " = t%d XOR 0xFFFFFFFF");
            break;
            
        case OP_ASSIGNMENT:
            // find lvalue target
            expr_target *lvalue_target = resolve_lvalue_target(expr->arg1);
            generate_expression_code(expr->arg2, lvalue_target);
            break;
        case OP_FUNC_CALL:
            generate_code_for_function_call(expr);
            break;

        default:
            ir.add_comment("\t(unhandled expr op %d (%s) follows)", expr->op, oper_debug_name(expr->op));
            t1 = expr_target_temp_reg(cg.next_reg_num());
            t2 = expr_target_temp_reg(cg.next_reg_num());
            if (expr->arg1) generate_expression_code(expr->arg1, t1);
            if (expr->arg2) generate_expression_code(expr->arg2, t2);
            break;
    }
}

