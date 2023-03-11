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


static ir_value *resolve_lvalue_expression(ir_listing *listing, expression *expr) {
    // e.g. we can have "a = 1", but also "a[entries[idx].a_offset] = 1"
    
    if (expr->op == OP_SYMBOL_NAME) {
        return new_ir_value_symbol(expr->value.str);

    } else if (expr->op == OP_POINTED_VALUE 
            || expr->op == OP_ARRAY_SUBSCRIPT
            || expr->op == OP_STRUCT_MEMBER_PTR
            || expr->op == OP_STRUCT_MEMBER_REF) {

        // generate expression, store result in temp reg
        ir_value *lvalue = new_ir_value_register(cg.next_reg_num());
        generate_expression_ir_code(listing, lvalue, expr);
        return lvalue;

    } else {
        error(expr->token->filename, expr->token->line_no, 
            "invalid lvalue expression \"%s\", expecting symbol, pointer or array element", 
            oper_debug_name(expr->op)
        );
    }
}

static void generate_ir_code_for_function_call(ir_listing *listing, ir_value *lvalue, expression *expr) {

    // need to find the func reference?  e.g. "(devices[0]->write)(h, 10, buffer)"
    // remember that C pushes args from right to left (IR opcode PUSH)
    // we can use "scall" IR code for direct symbol call,
    // or         "acall" IR code for calling the address stored in the symbol
    // don't forget to pop things after the call, using the IR code "POP <bytes>"
    // see http://web.archive.org/web/20151010192637/http://www.dound.com/courses/cs143/handouts/17-TAC-Examples.pdf

    // calculate function address
    ir_value *func_addr = resolve_lvalue_expression(listing, expr->arg1);

    // flatten arguments to be used
    #define MAX_FUNC_ARGS  16
    expression *arg_expressions[MAX_FUNC_ARGS];
    int argc = 0;
    expr->ops->flatten_func_call_args_to_array(expr, arg_expressions, 32, &argc);
    if (argc >= MAX_FUNC_ARGS) {
        error(expr->token->filename, expr->token->line_no, "only %d function arguments are supported, found %d", MAX_FUNC_ARGS, argc);
        return;
    }

    // prepare the ir_values array
    ir_value **ir_values_arr = malloc(sizeof(ir_value) * argc);
    for (int i = 0; i < argc; i++) {
        ir_values_arr[i] = new_ir_value_register(cg.next_reg_num());
        generate_expression_ir_code(listing, ir_values_arr[i], arg_expressions[i]);
    }

    listing->ops->add(listing, new_ir_function_call(lvalue, func_addr, argc, ir_values_arr));
}

static void generate_ir_code_for_binary_operation(ir_listing *listing, ir_value *lvalue, expression *expr) {

    ir_value *r1 = new_ir_value_register(cg.next_reg_num());
    generate_expression_ir_code(listing, r1, expr->arg1);

    ir_value *r2 = new_ir_value_register(cg.next_reg_num());
    generate_expression_ir_code(listing, r2, expr->arg2);
    
    ir_operation op = IR_NONE;
    switch (expr->op) {
        case OP_ADD: op = IR_ADD; break;
        case OP_SUB: op = IR_SUB; break;
        case OP_MUL: op = IR_MUL; break;
        case OP_DIV: op = IR_DIV; break;
        case OP_BITWISE_AND: op = IR_AND; break;
        case OP_BITWISE_OR: op = IR_OR; break;
        case OP_BITWISE_XOR: op = IR_XOR; break;
        case OP_BITWISE_NOT: op = IR_NOT; break;
        default:
            error(expr->token->filename, expr->token->line_no,
                "internal error, not a binary operation (%d, %s)", 
                expr->op, oper_debug_name(expr->op));
            break;
    }

    listing->ops->add(listing, new_ir_three_address_code(lvalue, r1, op, r2));
}

void generate_expression_ir_code(ir_listing *listing, ir_value *lvalue, expression *expr) {

    switch (expr->op) {
        case OP_NUM_LITERAL:
            listing->ops->add(listing, new_ir_assignment(lvalue, new_ir_value_immediate(expr->value.num)));
            break;

        case OP_CHR_LITERAL:
            listing->ops->add(listing, new_ir_assignment(lvalue, new_ir_value_immediate(expr->value.chr)));
            break;

        case OP_STR_LITERAL:
            char sym_name[16];
            sprintf(sym_name, "_str%d", cg.next_str_num());
            listing->ops->add(listing, new_ir_data_declaration(strlen(expr->value.str) + 1, expr->value.str, sym_name, IR_GLOBAL_RO));
            listing->ops->add(listing, new_ir_assignment(lvalue, new_ir_value_symbol(sym_name)));
            break;

        case OP_BOOL_LITERAL:
            listing->ops->add(listing, new_ir_assignment(lvalue, new_ir_value_immediate(expr->value.bln ? 1 : 0)));
            break;

        case OP_SYMBOL_NAME:
            // maybe the expectation is the contents of the variable and not the address????
            listing->ops->add(listing, new_ir_assignment(lvalue, new_ir_value_symbol(expr->value.str)));
            break;

        case OP_ADD: // fallthrough
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_BITWISE_AND:
        case OP_BITWISE_OR:
        case OP_BITWISE_XOR:
            generate_ir_code_for_binary_operation(listing, lvalue, expr);
            break;

        case OP_BITWISE_NOT:
            ir_value *rvalue = new_ir_value_register(cg.next_reg_num());
            generate_expression_ir_code(listing, rvalue, expr->arg2);
            listing->ops->add(listing, new_ir_unary_address_code(lvalue, IR_NOT, rvalue));
            break;

        case OP_FUNC_CALL:
            generate_ir_code_for_function_call(listing, lvalue, expr);
            break;

        case OP_ASSIGNMENT:
            // the provided lvale is ditched (e.g. a in "a = (b = c);")
            ir_value *lvalue_target = resolve_lvalue_expression(listing, expr->arg1);
            generate_expression_ir_code(listing, lvalue_target, expr->arg2);
            break;

        default:
            listing->ops->add(listing, new_ir_comment("(unhandled expresion (%s) follows)", oper_debug_name(expr->op)));
            ir_value *r1 = new_ir_value_register(cg.next_reg_num());
            ir_value *r2 = new_ir_value_register(cg.next_reg_num());
            if (expr->arg1) generate_expression_ir_code(listing, r1, expr->arg1);
            if (expr->arg2) generate_expression_ir_code(listing, r2, expr->arg2);
            break;
    }
}

