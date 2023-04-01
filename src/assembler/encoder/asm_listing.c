#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "asm_instruction.h"
#include "asm_listing.h"


static void _print(asm_listing *lst, FILE *stream);
static void _ensure_capacity(asm_listing *lst, int extra);
static void _set_next_label(asm_listing *lst, char *label, ...);
static void _set_next_comment(asm_listing *lst, char *comment, ...);
static void _add_comment(asm_listing *lst, char *comment, ...);
static void _add(asm_listing *lst, asm_instruction *instr);
static void _add_instr0(asm_listing *lst, enum opcode code); // e.g. NOP
static void _add_instr1(asm_listing *lst, enum opcode code, struct asm_operand *op);
static void _add_instr2(asm_listing *lst, enum opcode code, struct asm_operand *target_op, struct asm_operand *source_op);


static struct asm_listing_ops ops = {
    .print = _print,
    .set_next_label = _set_next_label,
    .set_next_comment = _set_next_comment,
    .add_comment = _add_comment,
    .add = _add,
    .add_instr0 = _add_instr0,
    .add_instr1 = _add_instr1,
    .add_instr2 = _add_instr2
};

asm_listing *new_asm_listing() {
    asm_listing *p = malloc(sizeof(asm_listing));

    p->capacity = 10;
    p->instruction_ptrs = malloc(sizeof(asm_instruction *) * p->capacity);
    p->length = 0;
    p->next_label = NULL;
    p->next_comment = NULL;
    p->ops = &ops;

    return p;
}

// for working with specific values, e.g. SUB SP, <bytes>
struct asm_operand *new_asm_operand_imm(int value) {
    struct asm_operand *op = malloc(sizeof(struct asm_operand));
    op->type = OT_IMMEDIATE;
    op->immediate = value;
    return op;
}

// for handling specific registers, e.g. BP, SP, AX
struct asm_operand *new_asm_operand_reg(enum gp_reg gp_reg_no) {
    struct asm_operand *op = malloc(sizeof(struct asm_operand));
    op->type = OT_REGISTER;
    op->reg = gp_reg_no;
    return op;
}

struct asm_operand *new_asm_operand_mem_by_sym(char *symbol_name) {
    struct asm_operand *op = malloc(sizeof(struct asm_operand));
    op->type = OT_MEM_OF_SYMBOL;
    op->symbol_name = strdup(symbol_name);
    return op;
}

struct asm_operand *new_asm_operand_mem_by_reg(enum gp_reg gp_reg_no, int offset) {
    struct asm_operand *op = malloc(sizeof(struct asm_operand));
    op->type = OT_MEM_POINTED_BY_REG;
    op->reg = gp_reg_no;
    op->offset = offset;
    return op;
}

static void _print(asm_listing *lst, FILE *stream) {
    str *str = new_str();
    
    struct asm_instruction *inst;

    for (int i = 0; i < lst->length; i++) {
        inst = lst->instruction_ptrs[i];

        if (inst->label != NULL)
            fprintf(stream, "%s:\n", inst->label);
        
        asm_instruction_to_str(inst, str);
        fprintf(stream, "    %s\n", str->buffer);
        str->v->clear(str);

        // perhaps allow some space between functions?
        if (inst->operation == OC_RET)
            fprintf(stream, "\n\n");
    }

    str->v->free(str);
}

static void _ensure_capacity(asm_listing *lst, int extra) {
    if (lst->length + extra < lst->capacity)
        return;

    while (lst->length + extra >= lst->capacity)
        lst->capacity *= 2;
    lst->instruction_ptrs = realloc(lst->instruction_ptrs, lst->capacity * sizeof(asm_instruction *));
}

static void _set_next_label(asm_listing *lst, char *label, ...) {
    char buffer[128];

    va_list vl;
    va_start(vl, label);
    vsnprintf(buffer, sizeof(buffer) - 1, label, vl);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(vl);

    lst->next_label = strdup(buffer);
}

static void _set_next_comment(asm_listing *lst, char *comment, ...) {
    char buffer[128];

    va_list vl;
    va_start(vl, comment);
    vsnprintf(buffer, sizeof(buffer) - 1, comment, vl);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(vl);

    // keep aside for next instruction, same as label
    lst->next_comment = strdup(buffer);
}

static void _add_comment(asm_listing *lst, char *comment, ...) {
    char buffer[128];

    va_list vl;
    va_start(vl, comment);
    vsnprintf(buffer, sizeof(buffer) - 1, comment, vl);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(vl);

    // add it as a standalone thing
    lst->next_comment = strdup(buffer);
    _add(lst, new_asm_instruction(OC_NONE));
}

static void _add_instr0(asm_listing *lst, enum opcode code) {
    _add(lst, new_asm_instruction(code));
}

static void _add_instr1(asm_listing *lst, enum opcode code, struct asm_operand *op) {
    _add(lst, new_asm_instruction_with_operand(code, op));
}

static void _add_instr2(asm_listing *lst, enum opcode code, struct asm_operand *target_op, struct asm_operand *source_op) {
    _add(lst, new_asm_instruction_with_operands(code, target_op, source_op));
}

void _add(asm_listing *lst, asm_instruction *instr) {
    instr->label = lst->next_label; // either null or not
    instr->comment = lst->next_comment; // null or not

    _ensure_capacity(lst, 1);
    lst->instruction_ptrs[lst->length] = instr;
    lst->length++;

    lst->next_label = NULL;
    lst->next_comment = NULL;
}
