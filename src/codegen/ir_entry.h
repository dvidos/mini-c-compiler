#pragma once
#include <stdbool.h>
#include <stdio.h>
#include "ir_value.h"

enum ir_entry_type {
    IR_FUNCTION_DEFINITION,
    IR_COMMENT,
    IR_LABEL,
    IR_DATA_DECLARATION,
    IR_THREE_ADDR_CODE,
    IR_FUNCTION_CALL,
    IR_CONDITIONAL_JUMP,
    IR_UNCONDITIONAL_JUMP,
    IR_FUNCTION_END,
};

typedef enum ir_data_storage {
    IR_GLOBAL,
    IR_GLOBAL_RO,
    IR_LOCAL,
    IR_RET_VAL,
} ir_data_storage;

typedef enum ir_comparison { 
    IR_EQ,
    IR_NE,
    IR_GT,
    IR_GE,
    IR_LT,
    IR_LE
} ir_comparison;

typedef enum ir_operation {
    IR_NONE = 0,
    IR_ADD, 
    IR_SUB, 
    IR_MUL, 
    IR_DIV, 
    IR_NEG, 
    IR_AND, 
    IR_OR, 
    IR_XOR, 
    IR_NOT,  // all bitwise
    IR_LSH, 
    IR_RSH, 
    IR_ADDR_OF,  // address-of operator
    IR_VALUE_AT, // pointer dereference (size?)
} ir_operation;

struct ir_entry_ops;


struct ir_entry_str_info {
    char *str;
};

struct ir_entry_func_def_info {
    char *func_name;
    struct ir_entry_func_arg_info *args_arr; // array of structs, left to right
    int args_len; // number of args
    int ret_val_size; // 0=void
};

struct ir_entry_func_arg_info {
    char *name; // e.g. "x"
    int size;   // e.g. 8
};

struct ir_entry_data_decl_info {
    int size;
    void *initial_data; // null for uninitialized
    char *symbol_name;
    ir_data_storage storage;
};

struct ir_entry_three_addr_code_info {
    ir_value *lvalue;
    ir_value *op1; // null for unary operators (not, neg, etc)
    ir_operation op;
    ir_value *op2;
};

struct ir_entry_function_call_info {
    ir_value *lvalue; // to store returned value
    ir_value *func_addr; // symbol or register or address etc.
    int args_len;
    ir_value **args_arr; // malloc()'d array of pointers
};

struct ir_entry_cond_jump_info {
    ir_value *v1;
    ir_comparison cmp;
    ir_value *v2;
    char *target_label;
};


typedef struct ir_entry {
    enum ir_entry_type type;
    union entry_union {
        struct ir_entry_func_def_info        function_def;
        struct ir_entry_str_info             comment;
        struct ir_entry_str_info             label;
        struct ir_entry_data_decl_info       data_decl;
        struct ir_entry_three_addr_code_info three_address_code;
        struct ir_entry_function_call_info   function_call;
        struct ir_entry_cond_jump_info       conditional_jump;
        struct ir_entry_str_info             unconditional_jump;
        struct {}                            function_end;
    } t;
    struct ir_entry_ops *ops;
} ir_entry;


ir_entry *new_ir_function_definition(char *func_name, struct ir_entry_func_arg_info *args_arr, int args_len, int ret_val_size);
ir_entry *new_ir_comment(char *fmt, ...);
ir_entry *new_ir_label(char *label_fmt, ...);
ir_entry *new_ir_data_declaration(int length, void *initial_data, char *symbol_name, ir_data_storage storage);
ir_entry *new_ir_assignment(ir_value *lvalue, ir_value *rvalue);
ir_entry *new_ir_unary_address_code(ir_value *lvalue, ir_operation op, ir_value *rvalue);
ir_entry *new_ir_three_address_code(ir_value *lvalue, ir_value *op1, ir_operation op, ir_value *op2);
ir_entry *new_ir_function_call(ir_value *lvalue, ir_value *func_addr, int args_count, ir_value **args_arr);
ir_entry *new_ir_conditional_jump(ir_value *v1, ir_comparison cmp, ir_value *v2, char *label_fmt, ...);
ir_entry *new_ir_unconditional_jump(char *label_fmt, ...);
ir_entry *new_ir_function_end();

struct ir_entry_ops {
    void (*print)(ir_entry *e, FILE *stream);
    void (*free)(ir_entry *e);
};
