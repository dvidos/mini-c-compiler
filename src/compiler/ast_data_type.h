#pragma once
#include "lexer/token.h"


typedef enum ast_type_family {
    TF_INT,
    TF_FLOAT,
    TF_CHAR,
    TF_BOOL,
    TF_VOID,
    TF_POINTER,
    TF_ARRAY,
} ast_type_family;

struct ast_data_type_ops;

// type of a variable or symbol
typedef struct ast_data_type {
    ast_type_family family; // int, char, etc
    struct ast_data_type *nested; // for pointer-of, or array-of etc.
    int array_size; // only for arrays
    char *string_repr; // calculated on the first to_string() call

    struct ast_data_type_ops *ops;
} ast_data_type;

struct ast_data_type_ops {
    int (*size_of)(ast_data_type *type);
    bool (*equals)(ast_data_type *a, ast_data_type *b);
    ast_data_type *(*clone)(ast_data_type *type);
    char *(*to_string)(ast_data_type *type); // no need to free
    void (*free)(ast_data_type *type);
};


ast_type_family data_type_family_for_token(token_type type);
ast_data_type *new_data_type(ast_type_family family, ast_data_type *nested);




