#pragma once
#include "operators.h"
#include "data_type.h"
#include "lexer/token.h"

typedef struct expression expression; // parsed expression for evaluation

typedef struct expression {
    oper op;
    expression *arg1;
    expression *arg2; // used as "next" for chains of func arguments

    union {
        char *str; // var name, func name, or string literal.
        long num;
        char chr;
        bool bln;
    } value;

    token *token;
    data_type *result_type;
} expression;


expression *create_expression(oper op, expression *arg1, expression *arg2, token *token);
expression *create_expr_symbol_name(char *name, token *token);
expression *create_expr_string_literal(char *str, token *token);
expression *create_expr_numeric_literal(char *number, token *token);
expression *create_expr_char_literal(char chr, token *token);
expression *create_expr_bool_literal(bool value, token *token);

data_type *expr_get_result_type(expression *expr);

