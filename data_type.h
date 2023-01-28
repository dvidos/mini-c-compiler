#pragma once
#include "lexer/token.h"


typedef enum type_family {
    TF_INT,
    TF_FLOAT,
    TF_CHAR,
    TF_BOOL,
    TF_VOID,
    TF_POINTER,
    TF_ARRAY,
} type_family;

// type of a variable or symbol
typedef struct data_type {
    type_family family; // int, char, etc
    struct data_type *nested; // for pointer-of, or array-of etc.
    int array_size; // only for arrays
    char *to_string;
} data_type;


type_family data_type_family_for_token(token_type type);
char *data_type_family_name(type_family t);
data_type *create_data_type(type_family family, data_type *nested);
data_type *clone_data_type(data_type *type);
void free_data_type(data_type *type);
bool data_types_equal(data_type *a, data_type *b);
char *data_type_to_string(data_type *type);