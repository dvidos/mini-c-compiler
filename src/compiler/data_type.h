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

struct data_type_ops;

// type of a variable or symbol
typedef struct data_type {
    type_family family; // int, char, etc
    struct data_type *nested; // for pointer-of, or array-of etc.
    int array_size; // only for arrays
    char *string_repr; // calculated on the first to_string() call

    struct data_type_ops *ops;
} data_type;

struct data_type_ops {
    int (*size_of)(data_type *type);
    bool (*equals)(data_type *a, data_type *b);
    data_type *(*clone)(data_type *type);
    char *(*to_string)(data_type *type); // no need to free
    void (*free)(data_type *type);
};


type_family data_type_family_for_token(token_type type);
data_type *new_data_type(type_family family, data_type *nested);




