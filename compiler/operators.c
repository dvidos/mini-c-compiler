#include "operators.h"

// see https://en.cppreference.com/w/c/language/operator_precedence
int get_precedence(oper op) {
    
    switch (op) {
        case OP_FUNC_CALL:
        case OP_ARRAY_SUBSCRIPT:
        case OP_STRUCT_MEMBER_PTR:
        case OP_STRUCT_MEMBER_REF:
        case OP_POST_INC:
        case OP_POST_DEC:
            return 1;
        case OP_UNARY_PLUS:
        case OP_UNARY_MINUS:
        case OP_LOGICAL_NOT:
        case OP_BINARY_NOT:
        case OP_PRE_INC:
        case OP_PRE_DEC:
        case OP_TYPE_CAST:
        case OP_POINTED_VALUE:
        case OP_ADDRESS_OF:
        case OP_SIZE_OF:
            return 2;
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
            return 3;
        case OP_ADD:
        case OP_SUB:
            return 4;
        case OP_LSHIFT:
        case OP_RSHIFT:
            return 5;
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
            return 6;
        case OP_EQ:
        case OP_NEQ:
            return 7;
        case OP_BITWISE_AND:
            return 8;
        case OP_BITWISE_OR:
            return 9;
        case OP_BITWISE_XOR:
            return 10;
        case OP_LOGICAL_AND:
            return 11;
        case OP_LOGICAL_OR:
            return 12;
        case OP_CONDITIONAL:
            return 13;
        case OP_ASSIGNMENT:
        case OP_ADD_ASSIGN:
        case OP_SUB_ASSIGN:
        case OP_MUL_ASSIGN:
        case OP_DIV_ASSIGN:
        case OP_MOD_ASSIGN:
        case OP_RSH_ASSIGN:
        case OP_LSH_ASSIGN:
        case OP_AND_ASSIGN:
        case OP_OR_ASSIGN:
        case OP_XOR_ASSIGN:
            return 14;
        case OP_COMMA:
            return 15;
    }
}