#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ir_entry.h"
#include "ir_value.h"


static void _print(ir_entry *e);
static void _free(ir_entry *e);

static struct ir_entry_ops ops = {
    .print = _print,
    .free = _free,
};

ir_entry *new_ir_comment(char *fmt, ...) {
    char buffer[128];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
    buffer[sizeof(buffer) - 1] = '\0';
    va_end(args);

    ir_entry *e = malloc(sizeof(ir_entry));
    e->type = IR_COMMENT;
    e->t.comment.str = strdup(buffer);
    e->ops = &ops;
    return e;
}

ir_entry *new_ir_label(char *fmt, ...) {
    char buffer[128];
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
    buffer[sizeof(buffer) - 1] = '\0';
    va_end(args);

    ir_entry *e = malloc(sizeof(ir_entry));
    e->type = IR_LABEL;
    e->t.label.str = strdup(buffer);
    e->ops = &ops;
    return e;
}

ir_entry *new_ir_data_declaration(int length, void *initial_data, char *symbol_name, ir_data_storage storage, int arg_no) {
    ir_entry *e = malloc(sizeof(ir_entry));
    e->type = IR_DATA_DECLARATION;
    e->t.data.length = length;
    if (initial_data == NULL) {
        e->t.data.initial_data = NULL;
    } else {
        e->t.data.initial_data = malloc(length);
        memcpy(e->t.data.initial_data, initial_data, length);
    }
    e->t.data.symbol_name = symbol_name == NULL ? NULL : strdup(symbol_name);
    e->t.data.storage = storage;
    e->t.data.arg_no = arg_no;
    e->ops = &ops;
    return e;
}

ir_entry *new_ir_assignment(ir_value *lvalue, ir_value *rvalue) {
    return new_ir_three_address_code(lvalue, NULL, IR_NONE, rvalue);
};

ir_entry *new_ir_unary_address_code(ir_value *lvalue, ir_operation op, ir_value *rvalue) {
    return new_ir_three_address_code(lvalue, NULL, IR_NONE, rvalue);
}

ir_entry *new_ir_three_address_code(ir_value *lvalue, ir_value *op1, ir_operation op, ir_value *op2) {
    ir_entry *e = malloc(sizeof(ir_entry));
    e->type = IR_THREE_ADDR_CODE;
    // not memcpy() we assume they were created by new_ir_value()
    e->t.three_address_code.lvalue = lvalue; 
    e->t.three_address_code.op1 = op1;
    e->t.three_address_code.op = op;
    e->t.three_address_code.op2 = op2;
    e->ops = &ops;
    return e;
}

ir_entry *new_ir_function_call(ir_value *lvalue, ir_value *func_addr, int args_len, ir_value **args_arr) {
    ir_entry *e = malloc(sizeof(ir_entry));
    e->type = IR_FUNCTION_CALL;
    e->t.function_call.lvalue = lvalue; 
    e->t.function_call.func_addr = func_addr;
    e->t.function_call.args_len = args_len;
    e->t.function_call.args_arr = args_arr; // we assume it was created for us.
    e->ops = &ops;
    return e;
}

ir_entry *new_ir_conditional_jump(ir_value *v1, ir_comparison cmp, ir_value *v2, char *label_fmt, ...) {
    char buffer[128];
    
    va_list args;
    va_start(args, label_fmt);
    vsnprintf(buffer, sizeof(buffer) - 1, label_fmt, args);
    buffer[sizeof(buffer) - 1] = '\0';
    va_end(args);

    ir_entry *e = malloc(sizeof(ir_entry));
    e->type = IR_CONDITIONAL_JUMP;
    // not memcpy() we assume they were created by new_ir_value()
    e->t.conditional_jump.v1 = v1;
    e->t.conditional_jump.cmp = cmp;
    e->t.conditional_jump.v2 = v2;
    e->t.conditional_jump.target_label = strdup(buffer);
    e->ops = &ops;
    return e;
}

ir_entry *new_ir_unconditional_jump(char *label_fmt, ...) {
    char buffer[128];
    
    va_list args;
    va_start(args, label_fmt);
    vsnprintf(buffer, sizeof(buffer) - 1, label_fmt, args);
    buffer[sizeof(buffer) - 1] = '\0';
    va_end(args);

    ir_entry *e = malloc(sizeof(ir_entry));
    e->type = IR_UNCONDITIONAL_JUMP;
    e->t.unconditional_jump.target_label = strdup(buffer);
    e->ops = &ops;
    return e;
}

static char *ir_operation_name(ir_operation op) {
    char *names[] = { "none", "add",  "sub",  "mul",  "div",  "neg", 
        "and", "or", "xor", "not", "lsh", "rsh", "addr_of", "value_at", };
    if (op >= 0 && op < sizeof(names) / sizeof(names[0]))
        return names[op];
    return "(unknown)";
}

static char *ir_data_storage_name(ir_data_storage s) {
    char *names[] = { "global", "global_ro", "local", "arg", "ret_val", };
    if (s >= 0 && s < sizeof(names) / sizeof(names[0]))
        return names[s];
    return "(unknown)";
}

static char *ir_comparison_name(ir_comparison cmp) {
    char *names[] = { "==", "!=", ">", ">=", "<", "<=" };
    if (cmp >= 0 && cmp < sizeof(names) / sizeof(names[0]))
        return names[cmp];
    return "(unknown)";
}

static void _print(ir_entry *e) {
    switch (e->type) {
        case IR_COMMENT:
            if (strlen(e->t.comment.str) > 0)
                printf("# %s", e->t.comment.str);
            else
                printf("%s", ""); // empty line
            break;

        case IR_LABEL:
            printf("%s:", e->t.label.str);
            break;

        case IR_DATA_DECLARATION:
            printf("\tdata %s \"%s\", %d bytes", 
                ir_data_storage_name(e->t.data.storage), 
                e->t.data.symbol_name, 
                e->t.data.length);
            if (e->t.data.initial_data != NULL) {
                if (e->t.data.length == 1)
                    printf(" = 0x%02x", *(unsigned char *)e->t.data.initial_data);
                else if (e->t.data.length == 2)
                    printf(" = 0x%04x", *(unsigned short *)e->t.data.initial_data);
                else if (e->t.data.length == 4)
                    printf(" = 0x%08x", *(unsigned short *)e->t.data.initial_data);
                else
                    printf(" ...");
            }
            break;

        case IR_THREE_ADDR_CODE:
            // can be a=c, a=!c, a=b+c, or even just c (func call)
            printf("\t");
            if (e->t.three_address_code.lvalue != NULL) {
                print_ir_value(e->t.three_address_code.lvalue);
                printf(" = ");
            }
            if (e->t.three_address_code.op1 != NULL) {
                print_ir_value(e->t.three_address_code.op1);
                printf(" ");
            }
            if (e->t.three_address_code.op != IR_NONE) {
                printf("%s ", ir_operation_name(e->t.three_address_code.op));
            }
            print_ir_value(e->t.three_address_code.op2);
            break;

        case IR_FUNCTION_CALL:
            printf("\t");
            if (e->t.function_call.lvalue != NULL) {
                print_ir_value(e->t.function_call.lvalue);
                printf(" = ");
            }
            printf("call ");
            print_ir_value(e->t.function_call.func_addr);
            if (e->t.function_call.args_len > 0) {
                printf(" passing ");
                for (int i = 0; i < e->t.function_call.args_len; i++) {
                    if (i > 0) printf(", ");
                    print_ir_value(e->t.function_call.args_arr[i]);
                }
            }
            break;

        case IR_CONDITIONAL_JUMP:
            printf("\t");
            printf("if ");
            print_ir_value(e->t.conditional_jump.v1);
            printf(" %s ", ir_comparison_name(e->t.conditional_jump.cmp));
            print_ir_value(e->t.conditional_jump.v2);
            printf(" goto %s", e->t.conditional_jump.target_label);
            break;

        case IR_UNCONDITIONAL_JUMP:
            printf("\t");
            printf("goto %s", e->t.unconditional_jump.target_label);
            break;
    }
}

static inline void free_nullable(char *p) {
    if (p != NULL) free(p);
}

static void _free(ir_entry *e) {
    switch (e->type) {
        case IR_COMMENT:
            free_nullable(e->t.comment.str);
            break;
        case IR_LABEL:
            free_nullable(e->t.label.str);
            break;
        case IR_DATA_DECLARATION:
            free_nullable(e->t.data.initial_data);
            free_nullable(e->t.data.symbol_name);
            break;
        case IR_THREE_ADDR_CODE:
            free_ir_value(e->t.three_address_code.lvalue);
            free_ir_value(e->t.three_address_code.op1);
            free_ir_value(e->t.three_address_code.op2);
            break;
        case IR_FUNCTION_CALL:
            free_ir_value(e->t.function_call.lvalue);
            free_ir_value(e->t.function_call.func_addr);
            for (int i = 0; i < e->t.function_call.args_len; i++)
                free_ir_value(e->t.function_call.args_arr[i]);
            free(e->t.function_call.args_arr);
            break;
        case IR_CONDITIONAL_JUMP:
            free_ir_value(e->t.conditional_jump.v1);
            free_ir_value(e->t.conditional_jump.v2);
            free_nullable(e->t.conditional_jump.target_label);
            break;
        case IR_UNCONDITIONAL_JUMP:
            free_nullable(e->t.unconditional_jump.target_label);
            break;
    }
    free(e);
}
