#include <stdio.h>
#include <stdarg.h>
#include "../err_handler.h"
#include "../ast_node.h"
#include "../statement.h"
#include "../scope.h"
#include "../symbol.h"
#include "analysis.h"


void perform_function_call_analysis(expression *call_expr) {
    // validate number and type of arguments passed
    // there may be commas or NULL for no args at all
}

void perform_expression_analysis(expression *expr) {
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
            symbol *s = scope_lookup(expr->value.str);
            if (s == NULL) {
                error(expr->token->filename, expr->token->line_no,
                    "symbol \"%s\" not declared", expr->value.str);
            }
            break;
        case OP_FUNC_CALL:
            perform_function_call_analysis(expr);
            break;
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
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
        case OP_BITWISE_NOT:
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
        case OP_NEQ:
            verify_expr_same_data_types(expr->arg1, expr->arg2, expr->token);
            break;
        case OP_LOGICAL_AND:
        case OP_LOGICAL_OR:
            verify_expr_result_boolean(expr->arg1);
            verify_expr_result_boolean(expr->arg2);
            break;
        case OP_LOGICAL_NOT:
            // this one is unary, it does not have 
            verify_expr_result_boolean(expr->arg1);
            break;
    }
}


void verify_expression_result_type(expression *expr, data_type *needed_type) {
    data_type *expr_result = get_expr_result_type(expr);
    if (expr_result == NULL) {
        error(expr->token->filename, expr->token->line_no, "expression returned type could not be calculated");
        return;
    }

    if (!data_types_equal(expr_result, needed_type)) {
        error(
            expr->token->filename, 
            expr->token->line_no,
            "expression returns a '%s', but a type of '%s' is required",
            data_type_to_string(expr_result),
            data_type_to_string(needed_type)
        );
    }
}

void verify_expr_result_integer(expression *expr) {
    if (expr == NULL)
        return;
    data_type *type = get_expr_result_type(expr);
    if (type->family != TF_INT || type->nested != NULL) {
        error(expr->token->filename, expr->token->line_no,
            "expression needs to produce integer type, instead of '%s'",
            data_type_to_string(type));
    }
}

void verify_expr_result_boolean(expression *expr) {
    if (expr == NULL)
        return;
    data_type *type = get_expr_result_type(expr);
    if (type->family != TF_BOOL || type->nested != NULL) {
        error(expr->token->filename, expr->token->line_no,
            "expression needs to produce integer type, instead of '%s'",
            data_type_to_string(type));
    }
}

void verify_expr_same_data_types(expression *expr1, expression *expr2, token *token) {
    if (expr1 == NULL && expr2 == NULL)
        return;
    if (expr1 == NULL || expr2 == NULL) {
        error(token->filename, token->line_no, "both operands need to be of same type, but one is missing");
        return;
    }
    data_type *type1 = get_expr_result_type(expr1);
    if (type1 == NULL) {
        error(expr1->token->filename, expr1->token->line_no, "cannot determine expression's data type");
        return;
    }
    data_type *type2 = get_expr_result_type(expr2);
    if (type2 == NULL) {
        error(expr2->token->filename, expr2->token->line_no, "cannot determine expression's data type");
        return;
    }
    if (data_types_equal(type1, type2)) {
        error(token->filename, token->line_no, 
            "operands need to be of same type, but first is '%s' and second is '%s'",
                data_type_to_string(type1),
                data_type_to_string(type2));
    }
}