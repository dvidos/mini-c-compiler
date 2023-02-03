#pragma once
#include "lexer/token.h"
#include "operators.h"
#include "data_type.h"
#include "statement.h"

typedef struct statement statement;


typedef struct var_declaration {

    char *var_name;

    // the declared data type, e.g. "int[]"
    data_type *data_type;

    // house keeping
    token *token;
    struct var_declaration *next; // for function arguments lists
} var_declaration;

var_declaration *create_var_declaration(data_type *data_type, char* var_name, token *token);



typedef struct func_declaration {

    char *func_name;

    // used to solve the data type of calling the function
    // and verify returned value types
    data_type *return_type;

    // can be null for functions without arguments
    var_declaration *args_list;

    // the list of contents
    statement *stmts_list;

    // housekeeping
    token *token;
    struct func_declaration *next;
} func_declaration;

func_declaration *create_func_declaration(data_type *return_type, char *func_name, var_declaration *args_list, statement *body, token *token);

