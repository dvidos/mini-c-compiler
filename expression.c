#include <stdlib.h>
#include "expression.h"
#include "operators.h"


expr_node *create_ast_expression(oper op, expr_node *arg1, expr_node *arg2) {
    expr_node *n = malloc(sizeof(expr_node));
    n->op = op;
    n->arg1 = arg1;
    n->arg2 = arg2;
    return n;
}

expr_node *create_ast_expr_name(char *name) {
    expr_node *n = create_ast_expression(OP_SYMBOL_NAME, NULL, NULL);
    n->value.str = name;
    return n;
}
expr_node *create_ast_expr_string_literal(char *str) {
    expr_node *n = create_ast_expression(OP_STR_LITERAL, NULL, NULL);
    n->value.str = str;
    return n;
}
expr_node *create_ast_expr_numeric_literal(char *number) {
    expr_node *n = create_ast_expression(OP_NUM_LITERAL, NULL, NULL);
    int base = 10;
    if (number[0] == '0' && number[1] != '\0') {
        if ((number[1] == 'x' || number[1] == 'X') && number[2] != '\0') {
            base = 16;
            number += 2;
        } else {
            base = 8;
            number += 1;
        }
    }
    n->value.num = strtol(number, NULL, base);
    return n;
}
expr_node *create_ast_expr_char_literal(char chr) {
    expr_node *n = create_ast_expression(OP_CHR_LITERAL, NULL, NULL);
    n->value.chr = chr;
    return n;
}

