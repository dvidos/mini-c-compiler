#pragma once
#include "operators.h"

typedef struct expr_node expr_node; // parsed expression for evaluation

typedef struct expr_node {
    oper op;
    expr_node *arg1;
    expr_node *arg2; // used as "next" for chains of func arguments

    union {
        char *str; // var name, func name, or string literal.
        long num;
        char chr;
        bool bln;
    } value;

} expr_node;


expr_node *create_ast_expression(oper op, expr_node *arg1, expr_node *arg2);
expr_node *create_ast_expr_name(char *name);
expr_node *create_ast_expr_string_literal(char *str);
expr_node *create_ast_expr_numeric_literal(char *number);
expr_node *create_ast_expr_char_literal(char chr);
expr_node *create_ast_expr_bool_literal(bool value);
