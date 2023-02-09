#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "options.h"
#include "data_type.h"

static int data_type_size_of(data_type *type);
static data_type *clone_data_type(data_type *type);
static void free_data_type(data_type *type);
static bool data_types_equal(data_type *a, data_type *b);
static char *data_type_to_string(data_type *type);



static struct data_type_ops ops = {
    .size_of = data_type_size_of,
    .equals = data_types_equal,
    .to_string = data_type_to_string,
    .clone = clone_data_type,
    .free = free_data_type,
};

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

data_type *create_data_type(type_family family, data_type *nested) {
    data_type *n = malloc(sizeof(data_type));
    n->family = family;
    n->nested = nested;
    n->array_size = 0;
    n->string_repr = 0;
    n->ops = &ops;
    return n;
}

static int data_type_size_of(data_type *type) {
    switch (type->family) {
        case TF_INT:
            return options.is_32_bits ? 4 : 8;
        case TF_FLOAT:
            return 8;
        case TF_CHAR:
            return 1;
        case TF_BOOL:
            return 1;
        case TF_VOID:
            return 0;
        case TF_POINTER:
            return options.is_32_bits ? 4 : 8;
        case TF_ARRAY:
            return type->array_size * data_type_size_of(type->nested);
    }
    
    return 0;
}



static data_type *clone_data_type(data_type *type) {
    if (type == NULL)
        return NULL;
    
    data_type *clone = malloc(sizeof(data_type));
    clone->family = type->family;
    clone->nested = clone_data_type(type->nested);
    clone->array_size = type->array_size;
    clone->string_repr = type->string_repr;
    clone->ops = &ops;
    return clone;
}

static void free_data_type(data_type *type) {
    if (type->nested != NULL)
        free_data_type(type);
    if (type->string_repr != NULL)
        free(type->string_repr);
    free(type);
}

static bool data_types_equal(data_type *a, data_type *b) {
    if ((a == b))
        return true;
    
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


static char *data_type_to_string(data_type *type) {
    if (type->string_repr != NULL)
        return type->string_repr;
    
    char *p = malloc(64); // careful when we introduce structs or func pointers
    p[0] = '\0';

    switch (type->family) {
        case TF_INT:   strcpy(p, "int");   break;
        case TF_FLOAT: strcpy(p, "float"); break;
        case TF_CHAR:  strcpy(p, "char");  break;
        case TF_BOOL:  strcpy(p, "bool");  break;
        case TF_VOID:  strcpy(p, "void");  break;

        case TF_POINTER:
            if (type->nested != NULL)
                strcat(p, data_type_to_string(type->nested));
            strcat(p, "*");
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

   type->string_repr = p;
   return type->string_repr;
}
