#include <stdio.h>
#include <stdarg.h>
#include "../../err_handler.h"
#include "../ast_declaration.h"
#include "../ast_statement.h"
#include "../scope.h"
#include "../src_symbol.h"
#include "analysis.h"


void validate_function_call_argument_type(ast_var_declaration *arg_decl, ast_expression *arg_expr, ast_func_declaration *func) {
    ast_data_type *expr_type = arg_expr->ops->get_data_type(arg_expr);
    if (!arg_decl->data_type->ops->equals(arg_decl->data_type, expr_type)) {
        error_at(arg_expr->token->filename, arg_expr->token->line_no,
            "argument '%s' of function '%s' needs to be of type '%s', but type '%s' was given",
            arg_decl->var_name,
            func->func_name,
            arg_decl->data_type->ops->to_string(arg_decl->data_type),
            expr_type->ops->to_string(expr_type));
    }
}

void perform_function_call_analysis(ast_expression *call_expr) {
    // validate number and type of arguments passed
    // there may be commas or NULL for no args at all
    if (call_expr == NULL)
        return;
    if (call_expr->arg1 == NULL) {
        error_at(call_expr->token->filename, call_expr->token->line_no, "call expression without arg1");
        return;
    }
    if (call_expr->arg1->op != OP_SYMBOL_NAME) {
        // we'll support pointers later
        error_at(call_expr->token->filename, call_expr->token->line_no, 
            "call expression arg1 expected symbol, got %s", oper_debug_name(call_expr->arg1->op));
        return;
    }
    src_symbol *sym = scope_lookup(call_expr->arg1->value.str);
    if (sym == NULL) {
        error_at(call_expr->token->filename, call_expr->token->line_no,
            "called function '%s' not found", call_expr->arg1->value.str);
        return;
    }

    // convert the COMMA tree into a flat array
    ast_expression *arg_exprs[32];
    int provided_args = 0;
    call_expr->ops->flatten_func_call_args_to_array(call_expr, arg_exprs, 32, &provided_args);
    int required_args = sym->func->ops->count_required_arguments(sym->func);

    // ensure at least the required arguments are provided
    if (provided_args < required_args) {
        error_at(call_expr->token->filename, call_expr->token->line_no,
            "function '%s' requires %d arguments, but only %d was/were given",
            sym->func->func_name, required_args, provided_args);
    }

    // verify the type of the arguments
    int i = 0;
    ast_var_declaration *arg_decl = sym->func->args_list;
    while (arg_decl != NULL && i < provided_args) {
        validate_function_call_argument_type(arg_decl, arg_exprs[i], sym->func);
        i++;
        arg_decl = arg_decl->next;
    }
}

void perform_expression_analysis(ast_expression *expr) {
    if (expr == NULL)
        return;
    
    // expression analyses are performed in a post-order manner,
    // to make sure the innermost expressions are verified first.
    perform_expression_analysis(expr->arg1);
    perform_expression_analysis(expr->arg2);

    oper op = expr->op;

    // see if identifiers are declared
    // see if the data types match what the operator expects or provides
    switch (op) {
        case OP_SYMBOL_NAME:
            src_symbol *s = scope_lookup(expr->value.str);
            if (s == NULL) {
                error_at(expr->token->filename, expr->token->line_no,
                    "symbol \"%s\" not declared", expr->value.str);
            }
            break;
        case OP_FUNC_CALL:
            perform_function_call_analysis(expr);
            break;
        case OP_ADD: // fallthroughs...
        case OP_SUB:
            verify_expr_result_integer_or_pointer(expr);
            break;
        case OP_MUL: // fallthroughs...
        case OP_DIV:
        case OP_MOD:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
        case OP_LSHIFT:
        case OP_RSHIFT:
        case OP_BITWISE_AND:
        case OP_BITWISE_OR:
        case OP_BITWISE_XOR:
            verify_expr_result_integer(expr->arg1);
            verify_expr_result_integer(expr->arg2);
            break;
        case OP_BITWISE_NOT: // fallthroughs
        case OP_PRE_INC:
        case OP_PRE_DEC:
        case OP_POST_INC:
        case OP_POST_DEC:
            // these are unary, they do not have arg2
            verify_expr_result_integer(expr->arg1);
            break;
        case OP_ASSIGNMENT:
            verify_expr_same_data_types(expr->arg1, expr->arg2, expr->token);
            break;
        case OP_EQ:
        case OP_NE:
            verify_expr_same_data_types(expr->arg1, expr->arg2, expr->token);
            break;
        case OP_LOGICAL_AND:
        case OP_LOGICAL_OR:
            verify_expr_result_boolean(expr->arg1);
            verify_expr_result_boolean(expr->arg2);
            break;
        case OP_LOGICAL_NOT:
            // this one is unary, it does not have arg2
            verify_expr_result_boolean(expr->arg1);
            break;
        case OP_ARRAY_SUBSCRIPT:
            // yeah... arg1 can be array or pointer... hence they are the same in C...
            verify_expr_result_array_or_pointer(expr->arg1);
            verify_expr_result_integer(expr->arg2);
            break;
    }
}


void verify_expression_result_type(ast_expression *expr, ast_data_type *needed_type) {
    ast_data_type *expr_result = expr->ops->get_data_type(expr);
    if (expr_result == NULL) {
        error_at(expr->token->filename, expr->token->line_no, "expression returned type could not be calculated");
        return;
    }

    if (!expr_result->ops->equals(expr_result, needed_type)) {
        error_at(
            expr->token->filename, 
            expr->token->line_no,
            "expression returns a '%s', but a type of '%s' is required",
            expr_result->ops->to_string(expr_result),
            needed_type->ops->to_string(needed_type)
        );
    }
}

void verify_expr_result_integer(ast_expression *expr) {
    if (expr == NULL)
        return;
    ast_data_type *type = expr->ops->get_data_type(expr);
    if (type->family != TF_INT || type->nested != NULL) {
        error_at(expr->token->filename, expr->token->line_no,
            "expression needs to produce integer type, instead of '%s'",
            type->ops->to_string(type));
    }
}

void verify_expr_result_integer_or_pointer(ast_expression *expr) {
    if (expr == NULL)
        return;
    ast_data_type *type = expr->ops->get_data_type(expr);
    if (!(type->family == TF_INT || type->family == TF_POINTER)) {
        error_at(expr->token->filename, expr->token->line_no,
            "expression needs to produce integer or pointer type, instead of '%s'",
            type->ops->to_string(type));
    }
}

void verify_expr_result_array_or_pointer(ast_expression *expr) {
    if (expr == NULL)
        return;
    ast_data_type *type = expr->ops->get_data_type(expr);
    if (!(type->family == TF_INT || type->family == TF_POINTER)) {
        error_at(expr->token->filename, expr->token->line_no,
            "expression needs to produce array or pointer type, instead of '%s'",
            type->ops->to_string(type));
    }
}

void verify_expr_result_boolean(ast_expression *expr) {
    if (expr == NULL)
        return;
    ast_data_type *type = expr->ops->get_data_type(expr);
    if (type->family != TF_BOOL || type->nested != NULL) {
        error_at(expr->token->filename, expr->token->line_no,
            "expression needs to produce boolean type, instead of '%s'",
            type->ops->to_string(type));
    }
}

void verify_expr_same_data_types(ast_expression *expr1, ast_expression *expr2, token *token) {
    if (expr1 == NULL && expr2 == NULL)
        return;
    if (expr1 == NULL || expr2 == NULL) {
        error_at(token->filename, token->line_no, "both operands need to be of same type, but one is missing");
        return;
    }
    ast_data_type *type1 = expr1->ops->get_data_type(expr1);
    if (type1 == NULL) {
        error_at(expr1->token->filename, expr1->token->line_no, "cannot determine expression's data type");
        return;
    }
    ast_data_type *type2 = expr2->ops->get_data_type(expr2);
    if (type2 == NULL) {
        error_at(expr2->token->filename, expr2->token->line_no, "cannot determine expression's data type");
        return;
    }
    if (!type1->ops->equals(type1, type2)) {
        error_at(token->filename, token->line_no, 
            "operands need to be of same type, but first is '%s' and second is '%s'",
            type1->ops->to_string(type1),
            type2->ops->to_string(type2));
    }
}