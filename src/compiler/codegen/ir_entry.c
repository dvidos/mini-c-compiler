#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../utils.h"
#include "ir_entry.h"
#include "ir_value.h"


static void _to_string(ir_entry *e, string *s);
static void _print(ir_entry *e, FILE *stream);
static void _foreach_ir_value(ir_entry *e, ir_value_visitor visitor, void *pdata, int idata);
static void _free(ir_entry *e);

static struct ir_entry_ops ops = {
    .to_string = _to_string,
    .print = _print,
    .foreach_ir_value = _foreach_ir_value,
    .free = _free,
};

ir_entry *new_ir_function_definition(char *func_name, struct ir_entry_func_arg_info *args_arr, int args_len, int ret_val_size) {
    ir_entry *e = malloc(sizeof(ir_entry));
    e->type = IR_FUNCTION_DEFINITION;
    e->t.function_def.func_name = strdup(func_name);
    e->t.function_def.args_arr = args_arr; // created for us.
    e->t.function_def.args_len = args_len;
    e->t.function_def.ret_val_size = ret_val_size;
    e->ops = &ops;
    return e;
}

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

ir_entry *new_ir_data_declaration(int length, void *initial_data, char *symbol_name, ir_data_storage storage) {
    ir_entry *e = malloc(sizeof(ir_entry));
    e->type = IR_DATA_DECLARATION;
    e->t.data_decl.size = length;
    if (initial_data == NULL) {
        e->t.data_decl.initial_data = NULL;
    } else {
        e->t.data_decl.initial_data = malloc(length);
        memcpy(e->t.data_decl.initial_data, initial_data, length);
    }
    e->t.data_decl.symbol_name = symbol_name == NULL ? NULL : strdup(symbol_name);
    e->t.data_decl.storage = storage;
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
    e->t.unconditional_jump.str = strdup(buffer);
    e->ops = &ops;
    return e;
}

ir_entry *new_ir_return(ir_value *ret_val) {
    ir_entry *e = malloc(sizeof(ir_entry));
    e->type = IR_RETURN;
    e->t.return_stmt.ret_val = ret_val; // may be null
    e->ops = &ops;
    return e;
}

ir_entry *new_ir_function_end() {
    ir_entry *e = malloc(sizeof(ir_entry));
    e->type = IR_FUNCTION_END;
    e->ops = &ops;
    return e;
}

static char *ir_operation_name(ir_operation op) {
    char *names[] = { "none", "+",  "-",  "*",  "/",  "neg", 
        "&", "|", "^", "~", "<<", ">>", "addr_of", "value_at", };
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

static void _to_string(ir_entry *e, string *s) {
    switch (e->type) {
        case IR_FUNCTION_DEFINITION:
            s->v->addf(s, "function \"%s\" (", e->t.function_def.func_name);
            for (int i = 0; i < e->t.function_def.args_len; i++) {
                if (i > 0) s->v->addf(s, ", ");
                s->v->addf(s, "%s:%d", 
                    e->t.function_def.args_arr[i].name, 
                    e->t.function_def.args_arr[i].size);
            }
            s->v->addf(s, ") -> %d", e->t.function_def.ret_val_size);
            break;

        case IR_COMMENT:
            s->v->addf(s, "# %s", e->t.comment.str);
            break;

        case IR_LABEL:
            s->v->addf(s, "%s:", e->t.label.str);
            break;

        case IR_DATA_DECLARATION:
            s->v->addf(s, "%s data \"%s\", %d bytes", 
            ir_data_storage_name(e->t.data_decl.storage), 
            e->t.data_decl.symbol_name, 
            e->t.data_decl.size);

            if (e->t.data_decl.initial_data != NULL) {
                int len = e->t.data_decl.size;
                char *ptr = e->t.data_decl.initial_data;

                if (len == 1)
                    s->v->addf(s, " = 0x%02x", *(unsigned char *)ptr);
                else if (len == 2)
                    s->v->addf(s, " = 0x%04x", *(unsigned short *)ptr);
                else if (len == 4)
                    s->v->addf(s, " = 0x%08x", *(unsigned short *)ptr);
                else {
                    // if all but the last are not zeros, it's a string
                    if (strchr(ptr, 0) == ptr + len - 1) {
                        s->v->adds(s, " = \"");
                        s->v->add_escaped(s, ptr);
                        s->v->adds(s, "\"");
                    }
                }
            }
            break;

        case IR_THREE_ADDR_CODE:
            // can be a=c, a=!c, a=b+c, or even just c (func call)
            ir_value_to_string(e->t.three_address_code.lvalue, s);
            s->v->addf(s, " = ");
            if (e->t.three_address_code.op1 != NULL) {
                ir_value_to_string(e->t.three_address_code.op1, s);
                s->v->addf(s, " ");
            }
            if (e->t.three_address_code.op != IR_NONE) {
                s->v->addf(s, "%s ", ir_operation_name(e->t.three_address_code.op));
            }
            ir_value_to_string(e->t.three_address_code.op2, s);
            break;

        case IR_FUNCTION_CALL:
            if (e->t.function_call.lvalue != NULL) {
                ir_value_to_string(e->t.function_call.lvalue, s);
                s->v->addf(s, " = ");
            }
            s->v->addf(s, "call ");
            ir_value_to_string(e->t.function_call.func_addr, s);
            if (e->t.function_call.args_len > 0) {
                s->v->addf(s, " passing ");
                for (int i = 0; i < e->t.function_call.args_len; i++) {
                    if (i > 0) s->v->addf(s, ", ");
                    ir_value_to_string(e->t.function_call.args_arr[i], s);
                }
            }
            break;

        case IR_CONDITIONAL_JUMP:
            s->v->addf(s, "if ");
            ir_value_to_string(e->t.conditional_jump.v1, s);
            s->v->addf(s, " %s ", ir_comparison_name(e->t.conditional_jump.cmp));
            ir_value_to_string(e->t.conditional_jump.v2, s);
            s->v->addf(s, " goto %s", e->t.conditional_jump.target_label);
            break;

        case IR_UNCONDITIONAL_JUMP:
            s->v->addf(s, "goto %s", e->t.unconditional_jump.str);
            break;

        case IR_RETURN:
            s->v->addf(s, "return");
            if (e->t.return_stmt.ret_val != NULL) {
                s->v->addf(s, " ");
                ir_value_to_string(e->t.return_stmt.ret_val, s);
            }
            break;

        case IR_FUNCTION_END:
            s->v->addf(s, "function end");
            break;
    }
}


static void _print(ir_entry *e, FILE *stream) {
    switch (e->type) {
        case IR_FUNCTION_DEFINITION:
            fprintf(stream, "function \"%s\" (", e->t.function_def.func_name);
            for (int i = 0; i < e->t.function_def.args_len; i++) {
                if (i > 0) fprintf(stream, ", ");
                fprintf(stream, "%s:%d", 
                    e->t.function_def.args_arr[i].name, 
                    e->t.function_def.args_arr[i].size);
            }
            fprintf(stream, ") -> %d", e->t.function_def.ret_val_size);
            break;

        case IR_COMMENT:
            fprintf(stream, "# %s", e->t.comment.str);
            break;

        case IR_LABEL:
            fprintf(stream, "%s:", e->t.label.str);
            break;

        case IR_DATA_DECLARATION:
            fprintf(stream, "    %s data \"%s\", %d bytes", 
            ir_data_storage_name(e->t.data_decl.storage), 
            e->t.data_decl.symbol_name, 
            e->t.data_decl.size);

            if (e->t.data_decl.initial_data != NULL) {
                int len = e->t.data_decl.size;
                char *ptr = e->t.data_decl.initial_data;

                if (len == 1)
                    fprintf(stream, " = 0x%02x", *(unsigned char *)ptr);
                else if (len == 2)
                    fprintf(stream, " = 0x%04x", *(unsigned short *)ptr);
                else if (len == 4)
                    fprintf(stream, " = 0x%08x", *(unsigned short *)ptr);
                else {
                    // if all but the last are not zeros, it's a string
                    if (strchr(ptr, 0) == ptr + len - 1) {
                        fprintf(stream, " = \"");
                        print_pretty(ptr, stream);
                        fprintf(stream, "\"");
                    }
                }
            }
            break;

        case IR_THREE_ADDR_CODE:
            // can be a=c, a=!c, a=b+c, or even just c (func call)
            fprintf(stream, "    ");
            print_ir_value(e->t.three_address_code.lvalue, stream);
            fprintf(stream, " = ");
            if (e->t.three_address_code.op1 != NULL) {
                print_ir_value(e->t.three_address_code.op1, stream);
                fprintf(stream, " ");
            }
            if (e->t.three_address_code.op != IR_NONE) {
                fprintf(stream, "%s ", ir_operation_name(e->t.three_address_code.op));
            }
            print_ir_value(e->t.three_address_code.op2, stream);
            break;

        case IR_FUNCTION_CALL:
            fprintf(stream, "    ");
            if (e->t.function_call.lvalue != NULL) {
                print_ir_value(e->t.function_call.lvalue, stream);
                fprintf(stream, " = ");
            }
            fprintf(stream, "call ");
            print_ir_value(e->t.function_call.func_addr, stream);
            if (e->t.function_call.args_len > 0) {
                fprintf(stream, " passing ");
                for (int i = 0; i < e->t.function_call.args_len; i++) {
                    if (i > 0) fprintf(stream, ", ");
                    print_ir_value(e->t.function_call.args_arr[i], stream);
                }
            }
            break;

        case IR_CONDITIONAL_JUMP:
            fprintf(stream, "    ");
            fprintf(stream, "if ");
            print_ir_value(e->t.conditional_jump.v1, stream);
            fprintf(stream, " %s ", ir_comparison_name(e->t.conditional_jump.cmp));
            print_ir_value(e->t.conditional_jump.v2, stream);
            fprintf(stream, " goto %s", e->t.conditional_jump.target_label);
            break;

        case IR_UNCONDITIONAL_JUMP:
            fprintf(stream, "    ");
            fprintf(stream, "goto %s", e->t.unconditional_jump.str);
            break;

        case IR_RETURN:
            fprintf(stream, "    return");
            if (e->t.return_stmt.ret_val != NULL) {
                fprintf(stream, " ");
                print_ir_value(e->t.return_stmt.ret_val, stream);
            }
            break;

        case IR_FUNCTION_END:
            fprintf(stream, "    function end");
            break;
    }
}

static void _foreach_ir_value(ir_entry *e, ir_value_visitor visitor, void *pdata, int idata) {
    int curr = 0;
    switch (e->type) {
        case IR_FUNCTION_DEFINITION: // fallthrough
        case IR_COMMENT:
        case IR_LABEL:
        case IR_DATA_DECLARATION:
        case IR_UNCONDITIONAL_JUMP:
        case IR_FUNCTION_END:
            break;
        case IR_FUNCTION_CALL:
            struct ir_entry_function_call_info *f = &e->t.function_call;
            visitor(f->lvalue, pdata, idata);
            visitor(f->func_addr, pdata, idata);
            for (int i = 0; i < f->args_len; i++)
                visitor(f->args_arr[i], pdata, idata);
            break;
        case IR_THREE_ADDR_CODE:
            struct ir_entry_three_addr_code_info *t = &e->t.three_address_code;
            visitor(t->lvalue, pdata, idata);
            visitor(t->op1, pdata, idata);
            visitor(t->op2, pdata, idata);
            break;
        case IR_CONDITIONAL_JUMP:
            struct ir_entry_cond_jump_info *j = &e->t.conditional_jump;
            visitor(j->v1, pdata, idata);
            visitor(j->v2, pdata, idata);
        case IR_RETURN:
            struct ir_entry_return_info *r = &e->t.return_stmt;
            if (r->ret_val != NULL)
                visitor(r->ret_val, pdata, idata);
            break;
    }
}

static inline void free_nullable(void *p) { if (p != NULL) free(p); }
static void _free(ir_entry *e) {
    switch (e->type) {
        case IR_FUNCTION_DEFINITION:
            free_nullable(e->t.function_def.func_name);
            free_nullable(e->t.function_def.args_arr);
        case IR_COMMENT:
            free_nullable(e->t.comment.str);
            break;
        case IR_LABEL:
            free_nullable(e->t.label.str);
            break;
        case IR_DATA_DECLARATION:
            free_nullable(e->t.data_decl.initial_data);
            free_nullable(e->t.data_decl.symbol_name);
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
            free_nullable(e->t.unconditional_jump.str);
            break;
        case IR_RETURN:
            free_ir_value(e->t.return_stmt.ret_val);
            break;
    }
    free(e);
}
