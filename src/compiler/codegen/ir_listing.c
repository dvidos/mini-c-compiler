#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ir_listing.h"
#include "../../utils/string.h"


static void _add(ir_listing *l, ir_entry *entry);
static void _print(ir_listing *l, FILE *stream);
static int _find_next_function_def(ir_listing *l, int start);
static void _run_statistics(ir_listing *l);
static int _get_register_last_usage(ir_listing *l, int reg_no);
static void _free(ir_listing *l);

static struct ir_listing_ops ops = {
    .add = _add,
    .print = _print,
    .find_next_function_def = _find_next_function_def,
    .run_statistics = _run_statistics,
    .get_register_last_usage = _get_register_last_usage,
    .free = _free
};

ir_listing *new_ir_listing() {
    ir_listing *l = malloc(sizeof(ir_listing));
    l->capacity = 10;
    l->length = 0;
    l->entries_arr = malloc(sizeof(ir_listing *) * l->capacity);
    l->ops = &ops;
    return l;
}

static void _add(ir_listing *l, ir_entry *entry) {
    if (l->length + 1 >= l->capacity) {
        l->capacity *= 2;
        l->entries_arr = realloc(l->entries_arr, sizeof(ir_listing *) * l->capacity);
    }

    l->entries_arr[l->length] = entry;
    l->length++;
}

static void _print(ir_listing *l ,FILE *stream) {
    string *s = new_string();
    for (int i = 0; i < l->length; i++) {
        ir_entry *e = l->entries_arr[i];
        if (e->type == IR_FUNCTION_DEFINITION)
            fprintf(stream, "\n");
        
        if (e->type == IR_LABEL)
            fprintf(stream, "%s:\n", e->t.label.str);
        else {
            if (e->type != IR_FUNCTION_DEFINITION)
                fprintf(stream, "    ");
                
            e->ops->to_string(e, s);
            fprintf(stream, "%s\n", s->buffer);
            s->v->clear(s);
        } 

        if (e->type == IR_FUNCTION_END) {
            fprintf(stream, "\n");
        }
    }
}

static int _find_next_function_def(ir_listing *l, int start) {
    for (int i = start; i < l->length; i++) {
        if (l->entries_arr[i]->type == IR_FUNCTION_DEFINITION)
            return i;
    }

    return -1;
}

static void _statistics_find_min_max_register_number(ir_value *v, void *pdata, int idata) {
    ir_listing *l = (ir_listing *)pdata;
    if (v != NULL && v->type == IR_TREG) {
        int reg_no = v->val.temp_reg_no;
        if (l->statistics.min_reg_no == 0 || reg_no < l->statistics.min_reg_no)
            l->statistics.min_reg_no = reg_no;
        
        if (l->statistics.max_reg_no == 0 || reg_no > l->statistics.max_reg_no)
            l->statistics.max_reg_no = reg_no;
    }
}

static void _statistics_find_each_register_last_index(ir_value *v, void *pdata, int idata) {
    ir_listing *l = (ir_listing *)pdata;
    int index = idata;
    if (v != NULL && v->type == IR_TREG) {
        int reg_no = v->val.temp_reg_no;
        if (index > l->statistics.reg_last_usage_arr[reg_no])
            l->statistics.reg_last_usage_arr[reg_no] = index;
    }
}

static void _run_statistics(ir_listing *l) {
    // find min/max registers, as well as last time each is mentioned
    l->statistics.min_reg_no = 0;
    l->statistics.max_reg_no = 0;
    l->statistics.regs_count = 0;
    if (l->statistics.reg_last_usage_arr != NULL)
        free(l->statistics.reg_last_usage_arr);
    l->statistics.reg_last_usage_arr = NULL;

    // first find how many registers
    for (int i = 0; i < l->length; i++) {
        ir_entry *e = l->entries_arr[i];
        e->ops->foreach_ir_value(e, _statistics_find_min_max_register_number, l, i);
    }

    // now make the array and run again to find last index
    l->statistics.regs_count = l->statistics.max_reg_no - l->statistics.min_reg_no + 1;
    l->statistics.reg_last_usage_arr = malloc(sizeof(int) * l->statistics.regs_count);
    memset(l->statistics.reg_last_usage_arr, 0, sizeof(int) * l->statistics.regs_count);
    for (int i = 0; i < l->length; i++) {
        ir_entry *e = l->entries_arr[i];
        e->ops->foreach_ir_value(e, _statistics_find_each_register_last_index, l, i);
    }

    // print the "last-used" array for sanity check
    // for (int regno = 0; regno < l->statistics.regs_count; regno++)
    //     printf("last usage of r%d is at index %d\n", regno, l->statistics.reg_last_usage_arr[regno]);
}

static int _get_register_last_usage(ir_listing *l, int reg_no) {
    if (reg_no < l->statistics.min_reg_no || reg_no > l->statistics.max_reg_no)
        return 0;
    return l->statistics.reg_last_usage_arr[reg_no];
}

static void _free(ir_listing *l) {
    for (int i = 0; i < l->length; i++) {
        ir_entry *e = l->entries_arr[i];
        e->ops->free(e);
    }
    if (l->statistics.reg_last_usage_arr != NULL)
        free(l->statistics.reg_last_usage_arr);
    free(l->entries_arr);
    free(l);
}
