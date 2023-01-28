#include <stdlib.h>
#include "expression.h"
#include "operators.h"
#include "data_type.h"
#include "symbol.h"
#include "scope.h"
#include "err_handler.h"
#include "lexer/token.h"


expression *create_expression(oper op, expression *arg1, expression *arg2, token *token) {
    expression *n = malloc(sizeof(expression));
    n->op = op;
    n->arg1 = arg1;
    n->arg2 = arg2;
    n->token = token;
    return n;
}

expression *create_symbol_name_expr(char *name, token *token) {
    expression *n = create_expression(OP_SYMBOL_NAME, NULL, NULL, token);
    n->value.str = name;
    return n;
}
expression *create_string_literal_expr(char *str, token *token) {
    expression *n = create_expression(OP_STR_LITERAL, NULL, NULL, token);
    n->value.str = str;
    return n;
}
expression *create_number_literal_expr(char *number, token *token) {
    expression *n = create_expression(OP_NUM_LITERAL, NULL, NULL, token);
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
expression *create_char_literal_expr(char chr, token *token) {
    expression *n = create_expression(OP_CHR_LITERAL, NULL, NULL, token);
    n->value.chr = chr;
    return n;
}

expression *create_bool_literal_expr(bool value, token *token) {
    expression *n = create_expression(OP_BOOL_LITERAL, NULL, NULL, token);
    n->value.bln = value;
    return n;
}

data_type *expr_get_result_type(expression *expr) {
    if (expr == NULL)
        return NULL;

    // if already discovered, no need to lose time
    if (expr->result_type != NULL)
        return expr->result_type;
    
    // let's discover it...
    oper op = expr->op;
    
    // first calculate terminal cases, we don't need nested expressions
    if (op == OP_STR_LITERAL) {
        expr->result_type = create_data_type(TF_POINTER, create_data_type(TF_CHAR, NULL));
    } else if (op == OP_NUM_LITERAL) {
        expr->result_type = create_data_type(TF_INT, NULL);
    } else if (op == OP_CHR_LITERAL) {
        expr->result_type = create_data_type(TF_CHAR, NULL);
    } else if (op == OP_BOOL_LITERAL) {
        expr->result_type = create_data_type(TF_BOOL, NULL);
    } else if (op == OP_SYMBOL_NAME) {
        // result will be whatever type the symbol is
        symbol *sym = scope_lookup(expr->value.str);
        if (sym == NULL)
            error(expr->token->filename, expr->token->line_no, "symbol \"%s\" not defined in current scope", expr->arg1);
        else
            expr->result_type = clone_data_type(sym->data_type);
    } else if (op == OP_FUNC_CALL) {
        // result will be whatever type the function returns
        if (!expr->arg1->op == OP_SYMBOL_NAME) {
            // for now we support symbols, lvalues (pointers) later
            error(expr->token->filename, expr->token->line_no, "func call expression did not have the symbol as arg1");
        } else {
            symbol *sym = scope_lookup(expr->arg1->value.str);
            if (sym == NULL) {
                error(expr->token->filename, expr->token->line_no, "symbol \"%s\" not defined in current scope", expr->arg1->value.str);
            } else {
                expr->result_type = clone_data_type(sym->data_type);
            }
        }
    } else if (op == OP_EQ || op == OP_NEQ
            || op == OP_LT || op == OP_LE || op == OP_GT || op == OP_GE
            || op == OP_LOGICAL_AND || op == OP_LOGICAL_OR || op == OP_LOGICAL_NOT) {
        // comparisons and logical operations
        expr->result_type = create_data_type(TF_BOOL, NULL);
    }

    // if discovered already, we are done.
    if (expr->result_type != NULL)
        return expr->result_type;
    
    // now we need to consult our arguments types.
    data_type *arg1_type = expr_get_result_type(expr->arg1);
    if (op == OP_POINTED_VALUE) {
        // return the nested type of arg1 type, i.e. *(of a char*) is a char
        if (arg1_type == NULL || arg1_type->nested == NULL) {
            error(expr->token->filename, expr->token->line_no, "pointer dereference, but pointee nested data type undefined");
        } else {
            expr->result_type = clone_data_type(arg1_type->nested);
        }
    } else if (op == OP_ARRAY_SUBSCRIPT) {
        // return the nested type of arg1 type, e.g. "int[]" will become int
        if (arg1_type == NULL || arg1_type->nested == NULL) {
            error(expr->token->filename, expr->token->line_no, "array element operation, but array item data type undefined");
        } else {
            expr->result_type = clone_data_type(arg1_type->nested);
        }
    } else if (op == OP_ADDRESS_OF) {
        // return a pointer to the type of arg1
        if (arg1_type == NULL) {
            error(expr->token->filename, expr->token->line_no, "address of opration, but target data type undefined");
        } else {
            expr->result_type = create_data_type(TF_POINTER, clone_data_type(arg1_type));
        }
    } else if (op == OP_BITWISE_NOT
            || op == OP_BITWISE_AND
            || op == OP_BITWISE_OR
            || op == OP_BITWISE_XOR
            || op == OP_POSITIVE_NUM
            || op == OP_NEGATIVE_NUM
            || op == OP_ADD
            || op == OP_SUB
            || op == OP_MUL
            || op == OP_DIV
            || op == OP_MOD) {
        // in theory int, but let's return whatever our first arg is.
        if (expr->arg1->result_type != NULL)
            expr->result_type = clone_data_type(expr->arg1->result_type);
    }
    
    // could be a warning
    if (expr->result_type == NULL)
        error(expr->token->filename, expr->token->line_no, 
            "could not derive returned data type of expression with operator %s", 
            oper_debug_name(op)
        );
    
    return expr->result_type;
}
