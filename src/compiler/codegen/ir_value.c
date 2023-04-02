#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ir_value.h"


ir_value *new_ir_value_symbol(char *symbol_name) {
    // symbols represent addresses in our IR, not values
    ir_value *v = malloc(sizeof(ir_value));
    v->type = IR_SYM;
    v->val.symbol_name = symbol_name;
    return v;
}

ir_value *new_ir_value_temp_reg(int temp_reg_no) {
    ir_value *v = malloc(sizeof(ir_value));
    v->type = IR_TREG;
    v->val.temp_reg_no = temp_reg_no;
    return v;
}

ir_value *new_ir_value_immediate(int value) {
    ir_value *v = malloc(sizeof(ir_value));
    v->type = IR_IMM;
    v->val.immediate = value;
    return v;
}

void print_ir_value(ir_value *v, FILE *stream) {
    if (v == NULL)
        fprintf(stream, "(null)");
    else if (v->type == IR_TREG)
        fprintf(stream, "r%d", v->val.temp_reg_no);
    else if (v->type == IR_SYM)
        fprintf(stream, "%s", v->val.symbol_name);
    else if (v->type == IR_IMM) {
        if (v->val.immediate >= 0 && v->val.immediate <= 9)
            fprintf(stream, "%d", v->val.immediate);
        else
            fprintf(stream, "0x%x", v->val.immediate);
    }
    else
        fprintf(stream, "(unknown)");
}

void ir_value_to_string(ir_value *v, str *s) {
    if (v == NULL)
        s->v->addf(s, "(null)");
    else if (v->type == IR_TREG)
        s->v->addf(s, "r%d", v->val.temp_reg_no);
    else if (v->type == IR_SYM)
        s->v->addf(s, "%s", v->val.symbol_name);
    else if (v->type == IR_IMM) {
        if (v->val.immediate >= 0 && v->val.immediate <= 9)
            s->v->addf(s, "%d", v->val.immediate);
        else
            s->v->addf(s, "0x%x", v->val.immediate);
    }
    else
        s->v->addf(s, "(unknown)");
}

void free_ir_value(ir_value *v) {
    if (v == NULL) return;
    if (v->type == IR_SYM && v->val.symbol_name != NULL)
        free(v->val.symbol_name);
    free(v);
}

