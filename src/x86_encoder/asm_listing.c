#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "instruction.h"
#include "asm_listing.h"


static void _print(asm_listing *lst, FILE *stream);
static void _ensure_capacity(asm_listing *lst, int extra);
static void _set_next_label(asm_listing *lst, char *label);
static void _add_comment(asm_listing *lst, char *comment, bool for_next_instruction);
static void _add_instr(asm_listing *lst, enum opcode code); // e.g. NOP
static void _add_instr_imm(asm_listing *lst, enum opcode code, u64 value); // e.g. PUSH 1
static void _add_instr_reg(asm_listing *lst, enum opcode code, enum reg reg); // e.g. PUSH EAX
static void _add_instr_sym(asm_listing *lst, enum opcode code, char *symbol); // e.g. CALL func1
static void _add_instr_reg_reg(asm_listing *lst, enum opcode code, enum reg reg1, enum reg reg2);
static void _add_instr_reg_imm(asm_listing *lst, enum opcode code, enum reg reg, u64 value);
static void _add_instr_reg_sym(asm_listing *lst, enum opcode code, enum reg reg, char *symbol_name);
static void _add_instr_imm_reg(asm_listing *lst, enum opcode code, u64 value, enum reg reg);
static void _add_instr_sym_reg(asm_listing *lst, enum opcode code, char *symbol_name, enum reg reg);


static struct asm_listing_ops ops = {
    .print = _print,
    .set_next_label = _set_next_label,
    .add_comment = _add_comment,
    .add_instr = _add_instr,
    .add_instr_imm = _add_instr_imm,
    .add_instr_reg = _add_instr_reg,
    .add_instr_sym = _add_instr_sym,
    .add_instr_reg_reg = _add_instr_reg_reg,
    .add_instr_reg_imm = _add_instr_reg_imm,
    .add_instr_reg_sym = _add_instr_reg_sym,
    .add_instr_imm_reg = _add_instr_imm_reg,
    .add_instr_sym_reg = _add_instr_sym_reg,
};

asm_listing *new_asm_listing() {
    asm_listing *p = malloc(sizeof(asm_listing));

    p->capacity = 10;
    p->instructions = malloc(sizeof(struct instruction) * p->capacity);
    p->length = 0;
    p->next_label = NULL;
    p->next_comment = NULL;
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
        
        if (inst->comment != NULL && inst->opcode == OC_NONE) {
            fprintf(stream, "; %s\n", inst->comment);
        } else {
            instruction_to_string(inst, buff, sizeof(buff));
            if (inst->comment == NULL) {
                fprintf(stream, "    %s\n", buff);
            } else {
                fprintf(stream, "    %-30s ; %s\n", buff, inst->comment);
            }
        }
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

static void _add_comment(asm_listing *lst, char *comment, bool for_next_instruction) {
    if (for_next_instruction) {
        // keep aside for next instruction, same as label
        lst->next_comment = strdup(comment); // we shall free it later on.
    } else {
        // add it as a standalone thing
        _ensure_capacity(lst, 1);
        struct instruction *inst = &lst->instructions[lst->length];

        inst->label = lst->next_label; // either null or not
        inst->opcode = OC_NONE;
        inst->op1.type = OT_NONE;
        inst->op2.type = OT_NONE;
        inst->comment = strdup(comment);

        lst->next_label = NULL;
        lst->length++;
    }
}

static void _add_instr(asm_listing *lst, enum opcode code) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_NONE;
    inst->op2.type = OT_NONE;
    inst->comment = lst->next_comment; // null or not

    lst->next_label = NULL;
    lst->next_comment = NULL;
    lst->length++;
}

static void _add_instr_imm(asm_listing *lst, enum opcode code, u64 value) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_IMMEDIATE;
    inst->op1.value = value;
    inst->op2.type = OT_NONE;
    inst->comment = lst->next_comment; // null or not

    lst->next_label = NULL;
    lst->next_comment = NULL;
    lst->length++;
}

static void _add_instr_reg(asm_listing *lst, enum opcode code, enum reg reg) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_REGISTER;
    inst->op1.value = reg;
    inst->op2.type = OT_NONE;
    inst->comment = lst->next_comment; // null or not

    lst->next_label = NULL;
    lst->next_comment = NULL;
    lst->length++;
}

static void _add_instr_sym(asm_listing *lst, enum opcode code, char *symbol) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_SYMBOL_MEM_ADDRESS;
    inst->op1.symbol_name = symbol;
    inst->op2.type = OT_NONE;
    inst->comment = lst->next_comment; // null or not

    lst->next_label = NULL;
    lst->next_comment = NULL;
    lst->length++;
}

static void _add_instr_reg_reg(asm_listing *lst, enum opcode code, enum reg reg1, enum reg reg2) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_REGISTER;
    inst->op1.value = reg1;
    inst->op2.type = OT_REGISTER;
    inst->op2.value = reg2;
    inst->comment = lst->next_comment; // null or not

    lst->next_label = NULL;
    lst->next_comment = NULL;
    lst->length++;
}

static void _add_instr_reg_imm(asm_listing *lst, enum opcode code, enum reg reg, u64 value) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_REGISTER;
    inst->op1.value = reg;
    inst->op2.type = OT_IMMEDIATE;
    inst->op2.value = value;
    inst->comment = lst->next_comment; // null or not

    lst->next_label = NULL;
    lst->next_comment = NULL;
    lst->length++;
}

static void _add_instr_reg_sym(asm_listing *lst, enum opcode code, enum reg reg, char *symbol_name) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_REGISTER;
    inst->op1.value = reg;
    inst->op2.type = OT_SYMBOL_MEM_ADDRESS;
    inst->op2.symbol_name = symbol_name;
    inst->comment = lst->next_comment; // null or not

    lst->next_label = NULL;
    lst->next_comment = NULL;
    lst->length++;
}

static void _add_instr_imm_reg(asm_listing *lst, enum opcode code, u64 value, enum reg reg) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_IMMEDIATE;
    inst->op1.value = value;
    inst->op2.type = OT_REGISTER;
    inst->op2.value = reg;
    inst->comment = lst->next_comment; // null or not

    lst->next_label = NULL;
    lst->next_comment = NULL;
    lst->length++;
}

static void _add_instr_sym_reg(asm_listing *lst, enum opcode code, char *symbol_name, enum reg reg) {
    _ensure_capacity(lst, 1);
    struct instruction *inst = &lst->instructions[lst->length];

    inst->label = lst->next_label; // either null or not
    inst->opcode = code;
    inst->op1.type = OT_SYMBOL_MEM_ADDRESS;
    inst->op1.symbol_name = symbol_name;
    inst->op2.type = OT_REGISTER;
    inst->op2.value = reg;
    inst->comment = lst->next_comment; // null or not

    lst->next_label = NULL;
    lst->next_comment = NULL;
    lst->length++;
}
