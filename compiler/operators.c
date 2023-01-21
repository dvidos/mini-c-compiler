#include "operators.h"
#include "token.h"


// see https://en.cppreference.com/w/c/language/operator_precedence
// for the shunting yard algorithm, 
// if two ops are equal precedence, the left associative take precedence
// precedence is numeric, larger numbers => higher precedence
int oper_precedence(oper op) {
    
    switch (op) {
        case OP_FUNC_CALL:
        case OP_ARRAY_SUBSCRIPT:
        case OP_STRUCT_MEMBER_PTR:
        case OP_STRUCT_MEMBER_REF:
        case OP_POST_INC:
        case OP_POST_DEC:
            return 99; // 1st

        case OP_NEGATIVE_NUM:
        case OP_POSITIVE_NUM:
        case OP_LOGICAL_NOT:
        case OP_BINARY_NOT:
        case OP_PRE_INC:
        case OP_PRE_DEC:
        case OP_TYPE_CAST:
        case OP_POINTED_VALUE:
        case OP_ADDRESS_OF:
        case OP_SIZE_OF:
            return 98; // 2

        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
            return 97; // 3

        case OP_ADD:
        case OP_SUB:
            return 96; // 4
        case OP_LSHIFT:
        case OP_RSHIFT:
            return 95; // 5
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
            return 94; // 6
        case OP_EQ:
        case OP_NEQ:
            return 93; // 7
        case OP_BITWISE_AND:
            return 92; // 8
        case OP_BITWISE_OR:
            return 91; // 9
        case OP_BITWISE_XOR:
            return 90; // 10
        case OP_LOGICAL_AND:
            return 89; // 11
        case OP_LOGICAL_OR:
            return 88; // 12
        case OP_CONDITIONAL:
            return 87; // 13
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
            return 86; // 14
        case OP_COMMA:
            return 85; // 15


        // these are leaf nodes, ideally not compared against other ops
        case OP_SYMBOL_NAME:
        case OP_STR_LITERAL:
        case OP_NUM_LITERAL: 
        case OP_CHR_LITERAL:
            return 1000;

        // this needs to have the lowest priority
        case OP_SENTINEL:
            return 1;
    }
}


// convert a token to a unary operator, if applicable
oper to_unary_operator(token_type type) {
    switch (type) {
        case TOK_LOGICAL_NOT: return OP_LOGICAL_NOT;
        case TOK_STAR: return OP_POINTED_VALUE;
        case TOK_AMPERSAND: return OP_ADDRESS_OF;
        case TOK_TILDE: return OP_BINARY_NOT;
        case TOK_MINUS_SIGN: return OP_NEGATIVE_NUM;
    }
    return OP_UNKNOWN;
}


// convert a token to a unary operator, if applicable
oper to_binary_operator(token_type type) {
    switch (type) {
        case TOK_PLUS_SIGN: return OP_ADD;
        case TOK_MINUS_SIGN: return OP_SUB;
        case TOK_STAR: return OP_MUL;
        case TOK_SLASH: return OP_DIV;
        case TOK_PERCENT: return OP_MOD;
        case TOK_BITWISE_AND: return OP_BITWISE_AND;
        case TOK_BITWISE_OR: return OP_BITWISE_OR;
        case TOK_CARET: return OP_BITWISE_XOR;
    }

    // this also used to deduct whether an operator is binary or not
    return OP_UNKNOWN;
}

