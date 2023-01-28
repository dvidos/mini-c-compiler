#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "data_type.h"


type_family data_type_family_for_token(token_type type) {
    switch (type) {
        case TOK_INT_KEYWORD:  return TF_INT;
        case TOK_FLOAT:        return TF_FLOAT;
        case TOK_CHAR_KEYWORD: return TF_CHAR;
        case TOK_BOOL:         return TF_BOOL;
        case TOK_VOID:         return TF_VOID;
        case TOK_STAR:         return TF_POINTER;
        case TOK_LBRACKET:     return TF_ARRAY;
    }
    return TF_INT;
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

data_type *create_data_type(type_family family, data_type *nested) {
    data_type *n = malloc(sizeof(data_type));
    n->family = family;
    n->nested = nested;
    n->array_size = 0;
    n->to_string = 0;
    return n;
}

data_type *clone_data_type(data_type *type) {
    if (type == NULL)
        return NULL;
    
    data_type *clone = malloc(sizeof(data_type));
    clone->family = type->family;
    clone->nested = clone_data_type(type->nested);
    clone->array_size = type->array_size;
    clone->to_string = type->to_string;
    return clone;
}

void free_data_type(data_type *type) {
    if (type->nested != NULL)
        free_data_type(type);
    if (type->to_string != NULL)
        free(type->to_string);
    free(type);
}

bool data_types_equal(data_type *a, data_type *b) {
    if ((a == b) || (a == NULL && b == NULL))
        return true;
    if (a == NULL || b == NULL)
        return false; // only one is null
    
    if (a->family != b->family)
        return false;
    
    // both families are the same
    if ((a->family == TF_ARRAY) && (a->array_size != b->array_size))
            return false;

    if (a->nested == NULL && b->nested == NULL)
        return true;
    else if (a->nested != NULL && b->nested != NULL)
        return data_types_equal(a->nested, b->nested);
    else
        return false; // only one is null
}


char *data_type_to_string(data_type *type) {
    if (type->to_string != NULL)
        return type->to_string;
    
    char *p = malloc(16); // careful when we introduce structs or func pointers
    p[0] = '\0';

    switch (type->family) {
        case TF_INT:   strcpy(p, "int");   break;
        case TF_FLOAT: strcpy(p, "float"); break;
        case TF_CHAR:  strcpy(p, "char");  break;
        case TF_BOOL:  strcpy(p, "bool");  break;
        case TF_VOID:  strcpy(p, "void");  break;

        case TF_POINTER:
            strcpy(p, "*");
            if (type->nested != NULL)
                strcat(p, data_type_to_string(type->nested));
            break;

        case TF_ARRAY:
            if (type->nested != NULL)
                strcpy(p, data_type_to_string(type->nested));
            sprintf(p + strlen(p), "[%d]", type->array_size);
            break;
        
        default:
            strcpy(p, "???");
            break;
   }

   type->to_string = p;
   return type->to_string;
}