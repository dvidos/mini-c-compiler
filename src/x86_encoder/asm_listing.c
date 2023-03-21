#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "asm_instruction.h"
#include "asm_listing.h"


static void _print(asm_listing *lst, FILE *stream);
static void _ensure_capacity(asm_listing *lst, int extra);
static void _set_next_label(asm_listing *lst, char *label);
static void _add_comment(asm_listing *lst, bool for_next_instruction, char *comment, ...);
static void _add_instr(asm_listing *lst, enum opcode code); // e.g. NOP
static void _add_instr1(asm_listing *lst, enum opcode code, struct asm_operand *op1);
static void _add_instr2(asm_listing *lst, enum opcode code, struct asm_operand *op1, struct asm_operand *op2);


static struct asm_listing_ops ops = {
    .print = _print,
    .set_next_label = _set_next_label,
    .add_comment = _add_comment,
    .add_instr = _add_instr,
    .add_instr1 = _add_instr1,
    .add_instr2 = _add_instr2
};

asm_listing *new_asm_listing() {
    asm_listing *p = malloc(sizeof(asm_listing));

    p->capacity = 10;
    p->instructions = malloc(sizeof(struct asm_instruction) * p->capacity);
    p->length = 0;
    p->next_label = NULL;
    p->next_comment = NULL;
    p->ops = &ops;

    return p;
}

// for working with specific values, e.g. SUB SP, <bytes>
struct asm_operand *new_imm_asm_operand(int value) {
    struct asm_operand *op = malloc(sizeof(struct asm_operand));
    op->type = OT_IMMEDIATE;
    op->immediate = value;
    return op;
}

// for handling specific registers, e.g. BP, SP, AX
struct asm_operand *new_reg_asm_operand(enum gp_reg gp_reg_no) {
    struct asm_operand *op = malloc(sizeof(struct asm_operand));
    op->type = OT_REGISTER;
    op->reg = gp_reg_no;
    return op;
}

struct asm_operand *new_mem_by_sym_asm_operand(char *symbol_name) {
    struct asm_operand *op = malloc(sizeof(struct asm_operand));
    op->type = OT_MEM_OF_SYMBOL;
    op->symbol_name = strdup(symbol_name);
    return op;
}

struct asm_operand *new_mem_by_reg_operand(enum gp_reg gp_reg_no, int offset) {
    struct asm_operand *op = malloc(sizeof(struct asm_operand));
    op->type = OT_MEM_POINTED_BY_REG;
    op->reg = gp_reg_no;
    op->offset = offset;
    return op;
}

static void _print(asm_listing *lst, FILE *stream) {
    struct asm_instruction *inst;
    char buff[128];

    for (int i = 0; i < lst->length; i++) {
        inst = &lst->instructions[i];
        if (inst->label != NULL)
            fprintf(stream, "%s:\n", inst->label);
        
        if (inst->comment != NULL && inst->opcode == OC_NONE) {
            fprintf(stream, "    ; %s\n", inst->comment);
        } else {
            instruction_to_string(inst, buff, sizeof(buff));
            if (inst->comment == NULL) {
                fprintf(stream, "    %s\n", buff);
            } else {
                fprintf(stream, "    %-20s ; %s\n", buff, inst->comment);
            }
        }

        // perhaps allow some space between functions?
        if (inst->opcode == OC_RET)
            fprintf(stream, "\n");
    }
}

static void _ensure_capacity(asm_listing *lst, int extra) {
    if (lst->length + extra < lst->capacity)
        return;

    while (lst->length + extra >= lst->capacity)
        lst->capacity *= 2;
    lst->instructions = realloc(lst->instructions, lst->capacity * sizeof(struct asm_instruction));
}

static void _set_next_label(asm_listing *lst, char *label) {
    lst->next_label = strdup(label); // we must free it later on.
}

static void _add_comment(asm_listing *lst, bool for_next_instruction, char *comment, ...) {
    char buffer[128];

    va_list vl;
    va_start(vl, comment);
    vsnprintf(buffer, sizeof(buffer) - 1, comment, vl);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(vl);

    if (for_next_instruction) {
        // keep aside for next instruction, same as label
        lst->next_comment = strdup(buffer);
    } else {
        // add it as a standalone thing
        _ensure_capacity(lst, 1);
        struct asm_instruction *inst = &lst->instructions[lst->length];

        inst->label = lst->next_label; // either null or not
        inst->opcode = OC_NONE;
        inst->op1 = NULL;
        inst->op2 = NULL;
        inst->comment = strdup(buffer);

        lst->next_label = NULL;
        lst->next_comment = NULL;
        lst->length++;
    }
}

static void _add_instr(asm_listing *lst, enum opcode code) {
    _ensure_capacity(lst, 1);
    struct asm_instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1 = NULL;
    inst->op2 = NULL;
    inst->comment = lst->next_comment; // null or not

    lst->next_label = NULL;
    lst->next_comment = NULL;
    lst->length++;
}

static void _add_instr1(asm_listing *lst, enum opcode code, struct asm_operand *op1) {
    _ensure_capacity(lst, 1);
    struct asm_instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1 = op1;
    inst->op2 = NULL;
    inst->comment = lst->next_comment; // null or not

    lst->next_label = NULL;
    lst->next_comment = NULL;
    lst->length++;
}

static void _add_instr2(asm_listing *lst, enum opcode code, struct asm_operand *op1, struct asm_operand *op2) {
    _ensure_capacity(lst, 1);
    struct asm_instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1 = op1;
    inst->op2 = op2;
    inst->comment = lst->next_comment; // null or not

    lst->next_label = NULL;
    lst->next_comment = NULL;
    lst->length++;
}

