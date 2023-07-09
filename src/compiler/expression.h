#pragma once
#include "operators.h"
#include "data_type.h"
#include "lexer/token.h"

typedef struct expression expression; // parsed expression for evaluation
struct expression_ops;

typedef struct expression {
    oper op;
    expression *arg1;
    expression *arg2; // used as "next" for chains of commas

    // some expressions contain literal values, e.g. "true" or "func1"
    union {
        const char *str; // var name, func name, or string literal.
        long num;
        char chr;
        bool bln;
    } value;

    // house keeping
    token *token;
    data_type *result_type;
    struct expression_ops *ops;
} expression;

struct expression_ops {
    data_type *(*get_data_type)(expression *expr);
    void (*flatten_func_call_args_to_array)(expression *call_expr, expression *arr[], int arr_size, int *args_count);
};

expression *create_expression(oper op, expression *arg1, expression *arg2, token *token);
expression *create_symbol_name_expr(const char *name, token *token);
expression *create_string_literal_expr(const char *str, token *token);
expression *create_number_literal_expr(const char *number, token *token);
expression *create_char_literal_expr(char chr, token *token);
expression *create_bool_literal_expr(bool value, token *token);

