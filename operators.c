#include "operators.h"
#include "lexer/token.h"


// see https://en.cppreference.com/w/c/language/operator_precedence
// for the shunting yard algorithm, 
// if two ops are equal precedence, the left associative take precedence
// precedence is numeric, larger numbers => higher precedence

struct operator_info {
    oper op;
    int precedence;
    bool unary;
    bool postfix;
    char *mnemonic;
};

struct operator_info operators_info_list[] = {
    // enum oper          prio  unary  postfix   mnemonic
    { OP_UNKNOWN,            0, false, false, "UNKNOWN" },  // to signify an unknown operator, when the token does not work

    { OP_FUNC_CALL,         29, false,  true, "CALL" },     // a()
    { OP_ARRAY_SUBSCRIPT,   29, false, true, "ELEM" },    // a[b]
    { OP_STRUCT_MEMBER_PTR, 29, false, true, "SPTR" },    // a->b
    { OP_STRUCT_MEMBER_REF, 29, false, true, "SMBM" },    // a.b
    { OP_POST_INC,          29, true,  true, "POSTINC" }, // a++
    { OP_POST_DEC,          29, true,  true, "POSTDEC" }, // a--
    { OP_POSITIVE_NUM,      28, true,  false, "NEG" },     // +123
    { OP_NEGATIVE_NUM,      28, true,  false, "POS" },     // -123
    { OP_LOGICAL_NOT,       28, true,  false, "NOT" },     // !a
    { OP_BINARY_NOT,        28, true,  false, "BIN_NOT" }, // ~a
    { OP_PRE_INC,           28, true,  false, "PREINC" },  // ++a
    { OP_PRE_DEC,           28, true,  false, "PREDEC" },  // --a
    { OP_TYPE_CAST,         28, false, false, "CAST" },    // (a)b
    { OP_POINTED_VALUE,     28, true,  false, "PTR" },     // *a
    { OP_ADDRESS_OF,        28, true,  false, "ADDR" },    // &a
    { OP_SIZE_OF,           28, true,  false, "SIZE" },    // sizeof(a) or sizeof a
    { OP_MUL,               27, false, false, "MUL" },     // a * b
    { OP_DIV,               27, false, false, "DIV" },     // a / b
    { OP_MOD,               27, false, false, "MOD" },     // a % b
    { OP_ADD,               26, false, false, "ADD" },     // a + b
    { OP_SUB,               26, false, false, "SUB" },     // a - b
    { OP_LSHIFT,            25, false, false, "LSH" },     // a << b
    { OP_RSHIFT,            25, false, false, "RSH" },     // a >> b
    { OP_LT,                24, false, false, "LT" },      // a < b
    { OP_LE,                24, false, false, "LE" },      // a <= b
    { OP_GT,                24, false, false, "GT" },      // a > b
    { OP_GE,                24, false, false, "GE" },      // a >= b
    { OP_EQ,                23, false, false, "EQ" },      // a == b
    { OP_NEQ,               23, false, false, "NEQ" },     // a != b
    { OP_BITWISE_AND,       22, false, false, "BIT_AND" }, // a & b
    { OP_BITWISE_OR,        21, false, false, "BIT_OR" },  // a | b
    { OP_BITWISE_XOR,       20, false, false, "BIT_XOR" }, // a ^ b
    { OP_LOGICAL_AND,       19, false, false, "AND" },     // a && b
    { OP_LOGICAL_OR,        18, false, false, "OR" },      // a || b
    { OP_CONDITIONAL,       17, false, false, "IIF" },     // a ? b : c
    { OP_ASSIGNMENT,        16, false, false, "ASSIGN" },  // a = b
    { OP_ADD_ASSIGN,        16, false, false, "ADD_ASN" }, // a += b
    { OP_SUB_ASSIGN,        16, false, false, "SUB_ASN" }, // a -= b
    { OP_MUL_ASSIGN,        16, false, false, "MUL_ASN" }, // a *= b
    { OP_DIV_ASSIGN,        16, false, false, "DIV_ASN" }, // a /= b
    { OP_MOD_ASSIGN,        16, false, false, "MOD_ASN" }, // a %= b
    { OP_RSH_ASSIGN,        16, false, false, "RSH_ASN" }, // a >>= b
    { OP_LSH_ASSIGN,        16, false, false, "LSH_ASN" }, // a <<= b
    { OP_AND_ASSIGN,        16, false, false, "AND_ASN" }, // a &= b
    { OP_OR_ASSIGN,         16, false, false, "OR_ASN" },  // a |= b
    { OP_XOR_ASSIGN,        16, false, false, "XOR_ASN" }, // a ^= b
    { OP_COMMA,             15, false, false, "COMMA" },   // a, b. ...

    { OP_SYMBOL_NAME,       1, false, false, "SYM" }, // caries a symbol name (var or func) value
    { OP_STR_LITERAL,       1, false, false, "STR" }, // carries a string value
    { OP_NUM_LITERAL,       1, false, false, "NUM" }, // carries a numeric value
    { OP_CHR_LITERAL,       1, false, false, "CHR" }, // carries a char value

    
    { OP_SENTINEL,          0, false, false, "SNTL" }, // for the shunting yard algorithm, this has the lowest priority of all
};

struct operator_info *operators_info_by_op[sizeof(operators_info_list) / sizeof(operators_info_list[0])];

// make operators a O(1) lookup, by using the enum value as an index.
void init_operators() {
    for (int i = 0; i < sizeof(operators_info_list) / sizeof(operators_info_list[0]); i++) {
        oper op = operators_info_list[i].op;
        operators_info_by_op[(int)op] = &operators_info_list[i];
    }
}

int oper_precedence(oper op) {
    return operators_info_by_op[(int)op]->precedence;
}

char *oper_debug_name(oper op) {
    return operators_info_by_op[(int)op]->mnemonic;
}

bool is_unary_operator(oper op) {
    return operators_info_by_op[(int)op]->unary;
}

// convert a token to a unary operator, if applicable
oper to_unary_operator(token_type type) {
    switch (type) {
        case TOK_LOGICAL_NOT: return OP_LOGICAL_NOT;
        case TOK_STAR:        return OP_POINTED_VALUE;
        case TOK_AMPERSAND:   return OP_ADDRESS_OF;
        case TOK_TILDE:       return OP_BINARY_NOT;
        case TOK_MINUS_SIGN:  return OP_NEGATIVE_NUM;
        case TOK_PLUS_SIGN:   return OP_POSITIVE_NUM;
        case TOK_INCREMENT:   return OP_PRE_INC;
        case TOK_DECREMENT:   return OP_PRE_DEC;
    }
    return OP_UNKNOWN;
}

// convert a token to a unary operator, if applicable
oper to_binary_operator(token_type type) {
    switch (type) {
        case TOK_PLUS_SIGN:    return OP_ADD;
        case TOK_MINUS_SIGN:   return OP_SUB;
        case TOK_STAR:         return OP_MUL;
        case TOK_SLASH:        return OP_DIV;
        case TOK_PERCENT:      return OP_MOD;
        case TOK_BITWISE_AND:  return OP_BITWISE_AND;
        case TOK_BITWISE_OR:   return OP_BITWISE_OR;
        case TOK_CARET:        return OP_BITWISE_XOR;
        case TOK_LESS_THAN:    return OP_LT;
        case TOK_LESS_EQUAL:   return OP_LE;
        case TOK_LARGER_THAN:  return OP_GT;
        case TOK_LARGER_EQUAL: return OP_GE;
        case TOK_COMMA:        return OP_COMMA;
        case TOK_ASSIGNMENT:   return OP_ASSIGNMENT;
        case TOK_LPAREN:       return OP_FUNC_CALL;
        case TOK_OPEN_BRACKET: return OP_ARRAY_SUBSCRIPT;
        case TOK_LOGICAL_OR:   return OP_LOGICAL_OR;
        case TOK_LOGICAL_AND:  return OP_LOGICAL_AND;
        case TOK_LOGICAL_NOT:  return OP_LOGICAL_NOT;
        case TOK_EQUAL_SIGN:   return OP_EQ;
        case TOK_LSHIFT:       return OP_LSHIFT;
        case TOK_RSHIFT:       return OP_RSHIFT;
    }
    return OP_UNKNOWN;
}

// convert a token to a unary operator, if applicable
oper to_postfix_operator(token_type type) {
    switch (type) {
        case TOK_LPAREN:       return OP_FUNC_CALL;
        case TOK_OPEN_BRACKET: return OP_ARRAY_SUBSCRIPT;
        case TOK_ARROW:        return OP_STRUCT_MEMBER_PTR;
        case TOK_DOT:          return OP_STRUCT_MEMBER_REF;
        case TOK_INCREMENT:    return OP_POST_INC;
        case TOK_DECREMENT:    return OP_POST_DEC;
    }
    return OP_UNKNOWN;
}

