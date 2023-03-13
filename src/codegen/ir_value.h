#pragma once
#include <stdbool.h>
#include <stdio.h>


enum ir_value_type { IR_REG, IR_SYM, IR_IMM };

typedef struct ir_value {
    enum ir_value_type type;
    union {
        char *symbol_name;
        int reg_no;
        int immediate;
    } val;
} ir_value;

ir_value *new_ir_value_symbol(char *symbol_name);
ir_value *new_ir_value_register(int reg_no);
ir_value *new_ir_value_immediate(int value);

// instead of ops struct, maybe hard-named values
void print_ir_value(ir_value *v, FILE *stream);
void free_ir_value(ir_value *v);

