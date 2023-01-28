#pragma once
#include "lexer/token.h"
#include "operators.h"
#include "data_type.h"
#include "expression.h"
#include "statement.h"

// ------------------------------------------------------------------

typedef struct ast_module_node     ast_module_node;     // source-file level
typedef struct func_declaration  func_declaration;  // function declaration or definition
typedef struct var_declaration   var_declaration;   // type + variable name
typedef struct statement  statement;  // what can be found in a block

// ------------------------------------------------------------------

typedef struct ast_module_node {

    // list of variables defined at module level
    statement *statements_list;

    // list of functions in the source file
    func_declaration *funcs_list;

} ast_module_node;

// ------------------------------------------------------------

typedef struct var_declaration {

    char *var_name;

    // the declared data type, e.g. "int[]"
    data_type *data_type;

    // house keeping
    token *token;
    struct var_declaration *next; // for function arguments lists
} var_declaration;

var_declaration *create_var_declaration(data_type *data_type, char* var_name, token *token);

// ------------------------------------------------------------

typedef struct func_declaration {

    char *func_name;

    // used to solve the data type of calling the function
    // and verify returned value types
    data_type *return_type;

    // can be null for functions without arguments
    var_declaration *args_list;

    // usually a block statement
    statement *body;

    // housekeeping
    token *token;
    struct func_declaration *next;
} func_declaration;

func_declaration *create_func_declaration(data_type *return_type, char *func_name, var_declaration *args_list, statement *body, token *token);


