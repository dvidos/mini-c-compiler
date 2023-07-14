#pragma once
#include "../lexer/token.h"

typedef enum ast_operator {
    OP_UNKNOWN,       // to signify an unknown operator, when the token does not work

    OP_FUNC_CALL,          // a()
    OP_ARRAY_SUBSCRIPT,    // a[b]
    OP_STRUCT_MEMBER_PTR,  // a->b
    OP_STRUCT_MEMBER_REF,  // a.b
    OP_POST_INC,           // a++
    OP_POST_DEC,           // a--
    OP_POSITIVE_NUM,       // +123
    OP_NEGATIVE_NUM,       // -123
    OP_LOGICAL_NOT,        // !a
    OP_BITWISE_NOT,        // ~a
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
    OP_NE,                 // !=
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
    OP_COMMA,              // a, b, 

    // for the shunting yard algorithm, this has the lowest priority of all
    OP_SENTINEL,

    // the following to allow AST leaf (terminal) nodes 
    OP_SYMBOL_NAME,   // caries a symbol name (var or func) value
    OP_STR_LITERAL,   // carries a string value
    OP_NUM_LITERAL,   // carries a numeric value
    OP_CHR_LITERAL,   // carries a char value
    OP_BOOL_LITERAL,  // carries a boolean value
} ast_operator;


void init_operators();

int oper_precedence(ast_operator op);
char *oper_debug_name(ast_operator op);
bool is_unary_operator(ast_operator op);
bool is_postfix_operator(ast_operator op);
ast_operator to_unary_operator(token_type type); // or OP_UNKNOWN if not supported
ast_operator to_binary_operator(token_type type); // or OP_UNKNOWN if not supported
ast_operator to_postfix_operator(token_type type); // or OP_INKNOWN if not supported

