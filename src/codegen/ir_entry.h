#pragma once
#include <stdbool.h>

// ----------------------------------------------------

enum ir_value_type { IR_REG, IR_SYM, IR_IMM };

typedef struct ir_value {
    enum ir_value_type type;
    union { char *symbol_name; int reg_no; int immediate; } val;
} ir_value;

ir_value *new_ir_value_symbol(char *symbol_name);
ir_value *new_ir_value_register(int reg_no);
ir_value *new_ir_value_immediate(int value);

// ----------------------------------------------------

enum ir_entry_type {
    IR_COMMENT,
    IR_LABEL,
    IR_DATA_DECLARATION,
    IR_THREE_ADDR_CODE,
    IR_FUNCTION_CALL,
    IR_CONDITIONAL_JUMP,
    IR_UNCONDITIONAL_JUMP,
};

typedef enum ir_comparison { 
    IR_EQ, IR_NE, IR_GT, IR_GE, IR_LT, IR_LE, 
} ir_comparison;

typedef enum ir_operation {
    IR_ADD, IR_SUB, IR_MUL, IR_DIV, IR_NEG, 
    IR_AND, IR_OR, IR_XOR, IR_NOT,  // all bitwise
    IR_LSH, IR_RSH, 
    IR_ADDR_OF, IR_ADDR_REF,
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
            int bytes;
            bool initialized;
            void *initial_data; // null for uninitialized
            char *symbol_name;
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

ir_entry *new_ir_comment(char *comment);
ir_entry *new_ir_label(char *label);
ir_entry *new_ir_data_decl(int bytes, bool initialized, void *initial_data);
ir_entry *new_ir_three_address_code(ir_value *lvalue, ir_value *op1, ir_operation op, ir_value *op2);
ir_entry *new_ir_unary_address_code(ir_value *lvalue, ir_operation op, ir_value *op2);
ir_entry *new_ir_function_call(ir_value *lvalue, ir_value *func_addr, int args_count, ir_value **args_arr);
ir_entry *new_ir_conditional_jump(ir_value *v1, ir_comparison cmp, ir_value *v2, char *target_label);
ir_entry *new_ir_unconditional_jump(char *target_label);

struct ir_entry_ops {
    void (*print)(ir_entry *e);
    void (*free)(ir_entry *e);
};
