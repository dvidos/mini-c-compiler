#include "stdlib.h"
#include "ast_node.h"
#include "token.h"

struct ast_data_type_node *create_ast_data_type_node(token *token, ast_data_type_node *nested) {
    enum type_family family;
    switch (token->type) {
        case TOK_INT: family = TF_INT; break;
        case TOK_CHAR: family = TF_CHAR; break;
        case TOK_VOID: family = TF_VOID; break;
        default: family = TF_INT;
    }
    
    ast_data_type_node *n = malloc(sizeof(ast_data_type_node));
    n->node_type = ANT_DATA_TYPE;
    n->family = family;
    n->nested = nested;
    return n;
}

ast_var_decl_node *create_ast_var_decl_node(ast_data_type_node *data_type, char* var_name) {
    ast_var_decl_node *n = malloc(sizeof(ast_var_decl_node));
    n->node_type = ANT_VAR_DECL;
    n->data_type = data_type;
    n->var_name = var_name;
    n->next = NULL;
    return n;
}

ast_func_decl_node *create_ast_func_decl_node(ast_data_type_node *return_type, char* func_name, ast_var_decl_node *args_list, void *body) {
    ast_func_decl_node *n = malloc(sizeof(ast_func_decl_node));
    n->node_type = ANT_FUNC_DECL;
    n->func_name = func_name;
    n->return_type = return_type;
    n->args_list = args_list;
    n->body = body;
    n->next = NULL;
    return n;
}

// #include <stddef.h>
// #include <stdio.h>
// #include "defs.h"
// #include "token.h"

// typedef enum {
//     DT_INT,
//     DT_CHAR
// } data_type;

// typedef struct {
//     data_type type;
//     // how are nested arrays depicted? or arrays of pointers?
//     int is_volatile;
//     int is_const;
//     int is_unsigned;
//     int is_short;
//     int is_long;
    
// } ast_type_node;

// typedef enum {
//     NT_DECLARATION,
//     NT_EXPRESSION,
//     NT_STATEMENT,
// } node_type;

// typedef struct {
//     node_type type; // should be present in all nnode sub-types, to allow casting
//     // then the rest
// } ast_node;

// typedef struct ast_if_node {
//     node_type type;
//     ast_node *condition;
//     ast_node *if_body;
//     ast_node *else_body;
// } ast_if_node;

// typedef struct ast_while_node {
//     node_type type;
//     ast_node *condition;
//     ast_node *body;
// } ast_if_node;


// typedef enum {
//     ET_STRING_LITERAL,
//     ET_NUMERIC_LITERAL,
//     ET_CHAR_LITERAL,
//     ET_BOOLEAN_LITERAL,
//     ET_ADD,
//     ET_SUBSTRACT,
//     ET_MULTIPLY,
//     ET_DIVIDE,
//     ET_LOGICAL_AND,
//     ET_LOGICAL_OR,
//     ET_LOGICAL_NOT,
//     ET_EQUALS,
//     ET_NOT_EQUALS,
// } expr_type;

// typedef struct expr_node {
//     expr_type type;
//     expr_node *arg1; // left or single argument
//     expr_node *arg2; // right arg or null for unary expression (e.g. address-of)
//     expr_node *list_next; // e.g. for function arguments
// } expr_node;

// // declaration for variables and functions
// ast_node *create_data_type_node(data_type type, char *name);
// ast_node *create_declaration_node(ast_type_node *type, char *name);
// ast_node *create_expression_node();
// ast_node *create_if_node(ast_node *condition, ast_node *if_body, ast_node *else_body);
// ast_node *create_while_node(ast_node *condition, ast_node *body);
// ast_node *create_break_node();
// ast_node *create_continue_node();
// ast_node *create_return_node();

