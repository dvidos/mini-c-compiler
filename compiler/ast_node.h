#pragma once
#include "token.h"


typedef enum ast_node_type {
    ANT_DATA_TYPE,
    ANT_VAR_DECL,
    ANT_FUNC_DECL,
} ast_node_type;

typedef struct ast_node {
    ast_node_type node_type; // all node types should have this first member
} ast_node;

// ------------------------------------------------------------

typedef enum type_family {
    TF_INT,
    TF_CHAR,
    TF_BOOL,
    TF_VOID,
} type_family;

// type of a variable or symbol
typedef struct ast_data_type_node {
    ast_node_type node_type; // to allow everything to be cast to ast_node

    type_family family; // int, char, etc
    struct ast_data_type_node *nested; // for pointer-of, or array-of etc.
} ast_data_type_node;

ast_data_type_node *create_ast_data_type_node(token *token, ast_data_type_node *nested);

// ------------------------------------------------------------

typedef struct ast_var_decl_node {
    ast_node_type node_type; // to allow everything to be cast to ast_node

    ast_data_type_node *data_type;
    char *var_name;
    // maybe an initialization expression could go here

    struct ast_var_decl_node *next; // for function arguments lists
} ast_var_decl_node;

ast_var_decl_node *create_ast_var_decl_node(ast_data_type_node *data_type, char* var_name);

// ------------------------------------------------------------

typedef struct ast_func_decl_node {
    ast_node_type node_type; // to allow everything to be cast to ast_node

    char *func_name;
    ast_data_type_node *return_type;
    ast_var_decl_node *args_list;
    void *body;

    struct ast_func_decl_node *next; // for function lists
} ast_func_decl_node;

ast_func_decl_node *create_ast_func_decl_node(ast_data_type_node *return_type, char *func_name, ast_var_decl_node *args_list, void *body);

// ------------------------------------------------------------






// typedef enum expression_type {
//     EX_LITERAL_VALUE,
//     EX_ADDITION,
//     EX_SUBTRACTION,
//     EX_MULTIPLICATION,
//     EX_DIVISION,
// } expression_type;

// typedef enum statement_type {
//     ST_DECL,
//     ST_EXPRESSION, // e.g. function call
//     ST_ASSIGNMENT,
//     ST_IF,
//     ST_WHILE,
//     ST_CONTINUE,
//     ST_BREAK,
//     ST_RETURN,
// } statement_type;



// // declaration of a variable or function
// struct declaration {
//     char *name;
//     struct type *type;
//     struct expression *value; // optional for initialization
//     struct statement *statements; // e.g. for functions

//     struct declaration *next;
// };

// // as simple as a literal num, or as complex as a tree
// struct expression {
//     // can be even a literal, a function call, operations etc.
//     // can be a tree of expressions
//     expression_type type;
//     struct expression *left; // or single, e.g. for the NOT expression
//     struct expression *right;
// };

// struct statement {
//     // statement type?
//     // assignment, for, while, if, return, etc
//     // ifs have one condition and two bodies, if and else
//     // whiles have one condition & one body
//     statement_type type;
    

//     struct statement *next;
// };

