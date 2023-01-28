#include <stdio.h>
#include <stdarg.h>
#include "../err_handler.h"
#include "../ast_node.h"
#include "../statement.h"
#include "../scope.h"
#include "../symbol.h"
#include "analysis.h"


void perform_expression_analysis(expression *expr) {
    if (expr == NULL)
        return;
    
    // expression analyses are performed in a post-order manner,
    // to make sure the innermost expressions are verified first.
    perform_expression_analysis(expr->arg1);
    perform_expression_analysis(expr->arg2);

    // see if identifiers are declared
    // see if the data types match what the operator expects or provides
    if (expr->op == OP_SYMBOL_NAME) {
        symbol *s = scope_lookup(expr->value.str);
        if (s == NULL) {
            error(
                expr->token->filename,
                expr->token->line_no,
                "symbol \"%s\" not declared",
                expr->value.str
            );
        }
    } else if (expr->op == OP_FUNC_CALL) {
        // should validate the type of each argument passed
        expr_get_result_type(expr);
    }

    /*  some ideas to explore
        - expression to be assigned must have the same type as the target lvalue
        - arguments passed in functions must match the argument's type
        - return value should match the return type of the function
        - all binary operators must have the same type left and right
        - (in)equality operators can not be applied to arrays, void.
        - (in)equality operators always return boolean
        - comparison > >= < <= operators can be applied to ints and return bool
          (could / should we apply to char? float? pointers?)
        - arithmetic operators + - * / etc apply to int/float and return int/float
    */
}


void verify_expression_type(expression *expr, data_type *needed_type) {
    data_type *expr_result = expr_get_result_type(expr);
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
