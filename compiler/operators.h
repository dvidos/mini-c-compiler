#pragma once


typedef enum oper {
    OP_FUNC_CALL,          // a()
    OP_ARRAY_SUBSCRIPT,    // a[b]
    OP_STRUCT_MEMBER_PTR,  // a->b
    OP_STRUCT_MEMBER_REF,  // a.b
    OP_POST_INC,           // a++
    OP_POST_DEC,           // a--
    OP_UNARY_PLUS,         // +123
    OP_UNARY_MINUS,        // -123
    OP_LOGICAL_NOT,        // !a
    OP_BINARY_NOT,         // ~a
    OP_PRE_INC,            // ++a
    OP_PRE_DEC,            // --a
    OP_TYPE_CAST,          // (b)a
    OP_POINTED_VALUE,      // *a
    OP_ADDRESS_OF,         // &a
    OP_SIZE_OF,            // sizeof(a) or sizeof a
    OP_MUL,                // *
    OP_DIV,                // /
    OP_MOD,                // %
    OP_ADD,                // +
    OP_SUB,                // -
    OP_LSHIFT,             // <<
    OP_RSHIFT,             // >>
    OP_LT,                 // <
    OP_LE,                 // <=
    OP_GT,                 // >
    OP_GE,                 // >=
    OP_EQ,                 // ==
    OP_NEQ,                // !=
    OP_BITWISE_AND,        // &
    OP_BITWISE_OR,         // |
    OP_BITWISE_XOR,        // ^
    OP_LOGICAL_AND,        // &&
    OP_LOGICAL_OR,         // ||
    OP_CONDITIONAL,        // a ? b : c
    OP_ASSIGNMENT,         // =
    OP_ADD_ASSIGN,         // +=
    OP_SUB_ASSIGN,         // -=
    OP_MUL_ASSIGN,         // *=
    OP_DIV_ASSIGN,         // /=
    OP_MOD_ASSIGN,         // %=
    OP_RSH_ASSIGN,         // >>=
    OP_LSH_ASSIGN,         // <<=
    OP_AND_ASSIGN,         // &=
    OP_OR_ASSIGN,          // |=
    OP_XOR_ASSIGN,         // ^=
    OP_COMMA               // a, b, 
} oper;

int get_precedence(operator op);