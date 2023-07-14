#pragma once
#include "ast_operator.h"
#include "ast_data_type.h"
#include "../lexer/token.h"

typedef struct ast_expression ast_expression; // parsed expression for evaluation
struct ast_expression_ops;

typedef struct ast_expression {
    ast_operator op;
    ast_expression *arg1;
    ast_expression *arg2; // used as "next" for chains of commas

    // some expressions contain literal values, e.g. "true" or "func1"
    union {
        const char *str; // var name, func name, or string literal.
        long num;
        char chr;
        bool bln;
    } value;

    // house keeping
    token *token;
    ast_data_type *result_type;
    struct ast_expression_ops *ops;
    mempool *mempool;
} ast_expression;

struct ast_expression_ops {
    ast_data_type *(*get_data_type)(ast_expression *expr);
    void (*flatten_func_call_args_to_array)(ast_expression *call_expr, ast_expression *arr[], int arr_size, int *args_count);
};

ast_expression *new_ast_expression(mempool *mp, ast_operator op, ast_expression *arg1, ast_expression *arg2, token *token);
ast_expression *new_ast_expression_symbol_name(mempool *mp, const char *name, token *token);
ast_expression *new_ast_expression_string_literal(mempool *mp, const char *str, token *token);
ast_expression *new_ast_expression_number_literal(mempool *mp, const char *number, token *token);
ast_expression *new_ast_expression_char_literal(mempool *mp, char chr, token *token);
ast_expression *new_ast_expression_bool_literal(mempool *mp, bool value, token *token);

