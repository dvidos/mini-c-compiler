#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ir_entry.h"


ir_value *new_ir_value_symbol(char *symbol_name) {
    ir_value *v = malloc(sizeof(ir_value));
    v->type = IR_SYM;
    v->val.symbol_name = symbol_name;
    return v;
}

ir_value *new_ir_value_register(int reg_no) {
    ir_value *v = malloc(sizeof(ir_value));
    v->type = IR_REG;
    v->val.reg_no = reg_no;
    return v;
}

ir_value *new_ir_value_immediate(int value) {
    ir_value *v = malloc(sizeof(ir_value));
    v->type = IR_IMM;
    v->val.immediate = value;
    return v;
}

