#pragma once
#include "lexer/token.h"


typedef struct data_type data_type;  // possibly nested data types

typedef enum type_family {
    TF_INT,
    TF_FLOAT,
    TF_CHAR,
    TF_BOOL,
    TF_VOID,
} type_family;

// type of a variable or symbol
typedef struct data_type {
    type_family family; // int, char, etc
    struct data_type *nested; // for pointer-of, or array-of etc.
} data_type;

data_type *create_ast_data_type_node(token *token, data_type *nested);
char *data_type_family_name(type_family t);

