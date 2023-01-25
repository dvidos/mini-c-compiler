#include <stdlib.h>
#include "data_type.h"


struct data_type *create_ast_data_type_node(token *token, data_type *nested) {
    enum type_family family;
    switch (token->type) {
        case TOK_INT_KEYWORD: family = TF_INT; break;
        case TOK_FLOAT: family = TF_FLOAT; break;
        case TOK_CHAR_KEYWORD: family = TF_CHAR; break;
        case TOK_BOOL: family = TF_BOOL; break;
        case TOK_VOID: family = TF_VOID; break;
        case TOK_STAR: family = TF_POINTER; break;
        case TOK_OPEN_BRACKET: family = TF_ARRAY; break;
        default: family = TF_INT;
    }
    
    data_type *n = malloc(sizeof(data_type));
    n->family = family;
    n->nested = nested;
    return n;
}

char *data_type_family_name(type_family t) {
    switch (t) {
        case TF_INT: return "int";
        case TF_FLOAT: return "float";
        case TF_CHAR: return "char";
        case TF_BOOL: return "bool";
        case TF_VOID: return "void";
        case TF_POINTER: return "pointer";
        case TF_ARRAY: return "array";
        default: return "*** unnamed ***";
    }
}


