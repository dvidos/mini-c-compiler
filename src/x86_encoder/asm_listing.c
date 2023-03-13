#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "instruction.h"
#include "asm_listing.h"


static void _print(asm_listing *lst, FILE *stream);
static void _ensure_capacity(asm_listing *lst, int extra);
static void _set_next_label(asm_listing *lst, char *label);
static void _add_single_instruction(asm_listing *lst, enum opcode code); // e.g. NOP
static void _add_instruction_with_immediate(asm_listing *lst, enum opcode code, u64 value); // e.g. PUSH 1
static void _add_instruction_with_register(asm_listing *lst, enum opcode code, enum reg reg); // e.g. PUSH EAX
static void _add_instruction_with_register_and_immediate(asm_listing *lst, enum opcode code, enum reg reg, u64 value);
static void _add_instruction_with_register_and_symbol(asm_listing *lst, enum opcode code, enum reg reg, char *symbol_name);

static struct asm_listing_ops ops = {
    .print = _print,
    .set_next_label = _set_next_label,
    .add_single_instruction = _add_single_instruction,
    .add_instruction_with_immediate = _add_instruction_with_immediate,
    .add_instruction_with_register = _add_instruction_with_register,
    .add_instruction_with_register_and_immediate = _add_instruction_with_register_and_immediate,
    .add_instruction_with_register_and_symbol = _add_instruction_with_register_and_symbol,
};

asm_listing *new_asm_listing() {
    asm_listing *p = malloc(sizeof(asm_listing));

    p->capacity = 10;
    p->instructions = malloc(sizeof(struct instruction) * p->capacity);
    p->length = 0;
    p->next_label = NULL;
    p->ops = &ops;

    return p;
}

static void _print(asm_listing *lst, FILE *stream) {
    struct instruction *inst;
    char buff[128];

    for (int i = 0; i < lst->length; i++) {
        inst = &lst->instructions[i];
        if (inst->label != NULL)
            fprintf(stream, "%s:\n", inst->label);
        
        instruction_to_string(inst, buff, sizeof(buff));
        fprintf(stream, "\t%s\n", buff);
    }
}

static void _ensure_capacity(asm_listing *lst, int extra) {
    if (lst->length + extra < lst->capacity)
        return;

    while (lst->length + extra >= lst->capacity)
        lst->capacity *= 2;
    lst->instructions = realloc(lst->instructions, lst->capacity * sizeof(struct instruction));
}

static void _set_next_label(asm_listing *lst, char *label) {
    lst->next_label = strdup(label); // we must free it later on.
}

static void _add_single_instruction(asm_listing *lst, enum opcode code) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_NONE;
    inst->op2.type = OT_NONE;

    lst->next_label = NULL;
    lst->length++;
}

static void _add_instruction_with_immediate(asm_listing *lst, enum opcode code, u64 value) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_IMMEDIATE;
    inst->op1.value = value;
    inst->op2.type = OT_NONE;

    lst->next_label = NULL;
    lst->length++;
}

static void _add_instruction_with_register(asm_listing *lst, enum opcode code, enum reg reg) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_REGISTER;
    inst->op1.value = reg;
    inst->op2.type = OT_NONE;

    lst->next_label = NULL;
    lst->length++;
}

static void _add_instruction_with_register_and_immediate(asm_listing *lst, enum opcode code, enum reg reg, u64 value) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_REGISTER;
    inst->op1.value = reg;
    inst->op2.type = OT_IMMEDIATE;
    inst->op2.value = value;

    lst->next_label = NULL;
    lst->length++;
}

static void _add_instruction_with_register_and_symbol(asm_listing *lst, enum opcode code, enum reg reg, char *symbol_name) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_REGISTER;
    inst->op1.value = reg;
    inst->op2.type = OT_SYMBOL_MEM_ADDRESS;
    inst->op2.symbol_name = symbol_name;

    lst->next_label = NULL;
    lst->length++;
}

