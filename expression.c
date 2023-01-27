#include <stdlib.h>
#include "expression.h"
#include "operators.h"
#include "data_type.h"
#include "symbol.h"
#include "scope.h"


expr_node *create_ast_expression(oper op, expr_node *arg1, expr_node *arg2, token *token) {
    expr_node *n = malloc(sizeof(expr_node));
    n->op = op;
    n->arg1 = arg1;
    n->arg2 = arg2;
    n->token = token;
    return n;
}

expr_node *create_ast_expr_name(char *name, token *token) {
    expr_node *n = create_ast_expression(OP_SYMBOL_NAME, NULL, NULL, token);
    n->value.str = name;
    return n;
}
expr_node *create_ast_expr_string_literal(char *str, token *token) {
    expr_node *n = create_ast_expression(OP_STR_LITERAL, NULL, NULL, token);
    n->value.str = str;
    return n;
}
expr_node *create_ast_expr_numeric_literal(char *number, token *token) {
    expr_node *n = create_ast_expression(OP_NUM_LITERAL, NULL, NULL, token);
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
expr_node *create_ast_expr_char_literal(char chr, token *token) {
    expr_node *n = create_ast_expression(OP_CHR_LITERAL, NULL, NULL, token);
    n->value.chr = chr;
    return n;
}

expr_node *create_ast_expr_bool_literal(bool value, token *token) {
    expr_node *n = create_ast_expression(OP_BOOL_LITERAL, NULL, NULL, token);
    n->value.bln = value;
    return n;
}

data_type *get_expression_result_type(expr_node *expr) {
    oper op = expr->op;
    switch (op) {
        case OP_STR_LITERAL:
            return create_data_type(TF_POINTER, create_data_type(TF_CHAR, NULL));
        case OP_NUM_LITERAL:
            return create_data_type(TF_INT, NULL);
        case OP_CHR_LITERAL:
            return create_data_type(TF_CHAR, NULL);
        case OP_BOOL_LITERAL:
            return create_data_type(TF_BOOL, NULL);

        case OP_SYMBOL_NAME:
        case OP_FUNC_CALL:
            // we have to lookup the symbol type...
            // or whatever that function returns...
            symbol *sym = scope_lookup((char *)expr->arg1);
            if (sym == NULL) return NULL;
            return clone_data_type(sym->data_type);

        case OP_ADDRESS_OF:
        case OP_ARRAY_SUBSCRIPT:
            // for pointers, arg1 is the pointed expression, whose nested type
            //     is the pointed thing.
            // for arrays, arg1 is the array, whose nested type
            //     is the array element.
            // keping it simple for now.
            data_type *arg_type = get_expression_result_type(expr->arg1);
            data_type *result = clone_data_type(arg_type->nested);
            free_data_type(arg_type);
            return result;
        
        case OP_BITWISE_NOT:
        case OP_BITWISE_AND:
        case OP_BITWISE_OR:
        case OP_BITWISE_XOR:
        case OP_POSITIVE_NUM:
        case OP_NEGATIVE_NUM:
            // in theory int, but let's return whatever our first arg is.
            return get_expression_result_type(expr->arg1);

        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
            // in theory int, but let's return whatever our first arg is.
            return get_expression_result_type(expr->arg1);

        case OP_EQ:
        case OP_NEQ:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
        case OP_LOGICAL_AND:
        case OP_LOGICAL_OR:
        case OP_LOGICAL_NOT:
            // comparisons and logical operations
            return create_data_type(TF_BOOL, NULL);
    }
    return NULL;
}
