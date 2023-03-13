#pragma once
#include <stdbool.h>
#include <stdio.h>
#include "ir_value.h"

enum ir_entry_type {
    IR_COMMENT,
    IR_LABEL,
    IR_DATA_DECLARATION,
    IR_THREE_ADDR_CODE,
    IR_FUNCTION_CALL,
    IR_CONDITIONAL_JUMP,
    IR_UNCONDITIONAL_JUMP,
};

typedef enum ir_data_storage {
    IR_GLOBAL,
    IR_GLOBAL_RO,
    IR_LOCAL,
    IR_FUNC_ARG,
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

typedef struct ir_entry {
    enum ir_entry_type type;
    union entry_union {
        struct {
            char *str;
        } comment;
        struct {
            char *str;
        } label;
        struct {
            int length;
            void *initial_data; // null for uninitialized
            char *symbol_name;
            ir_data_storage storage;
            int arg_no; // for local arguments
        } data;
        struct {
            ir_value *lvalue;
            ir_value *op1; // null for unary operators (not, neg, etc)
            ir_operation op;
            ir_value *op2;
        } three_address_code;
        struct {
            ir_value *lvalue; // to store returned value
            ir_value *func_addr; // symbol or register or address etc.
            int args_len;
            ir_value **args_arr; // malloc()'d array of pointers
        } function_call;
        struct {
            ir_value *v1;
            ir_comparison cmp;
            ir_value *v2;
            char *target_label;
        } conditional_jump;
        struct {
            char *target_label;
        } unconditional_jump;
    } t;

    struct ir_entry_ops *ops;
} ir_entry;

ir_entry *new_ir_comment(char *fmt, ...);
ir_entry *new_ir_label(char *label_fmt, ...);
ir_entry *new_ir_data_declaration(int length, void *initial_data, char *symbol_name, ir_data_storage storage, int arg_no);
ir_entry *new_ir_assignment(ir_value *lvalue, ir_value *rvalue);
ir_entry *new_ir_unary_address_code(ir_value *lvalue, ir_operation op, ir_value *rvalue);
ir_entry *new_ir_three_address_code(ir_value *lvalue, ir_value *op1, ir_operation op, ir_value *op2);
ir_entry *new_ir_function_call(ir_value *lvalue, ir_value *func_addr, int args_count, ir_value **args_arr);
ir_entry *new_ir_conditional_jump(ir_value *v1, ir_comparison cmp, ir_value *v2, char *label_fmt, ...);
ir_entry *new_ir_unconditional_jump(char *label_fmt, ...);

struct ir_entry_ops {
    void (*print)(ir_entry *e, FILE *stream);
    void (*free)(ir_entry *e);
};
