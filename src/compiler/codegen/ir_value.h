#pragma once
#include <stdbool.h>
#include <stdio.h>
#include "../../utils/string.h"


enum ir_value_type { IR_TREG, IR_SYM, IR_IMM };

typedef struct ir_value {
    enum ir_value_type type;
    union {
        const char *symbol_name;
        int temp_reg_no;
        int immediate;
    } val;
} ir_value;

ir_value *new_ir_value_symbol(const char *symbol_name);
ir_value *new_ir_value_temp_reg(int temp_reg_no);
ir_value *new_ir_value_immediate(int value);

// instead of ops struct, maybe hard-named values
void print_ir_value(ir_value *v, FILE *stream);
void ir_value_to_string(ir_value *v, string *s);
void free_ir_value(ir_value *v);

