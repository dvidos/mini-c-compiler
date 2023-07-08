#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../utils/data_structs.h"
#include "asm_instruction.h"
#include "asm_listing.h"


static void asm_listing_print(asm_listing *lst, FILE *stream);
static void asm_listing_ensure_capacity(asm_listing *lst, int extra);
static void asm_listing_set_next_label(asm_listing *lst, char *label, ...);
static void asm_listing_set_next_comment(asm_listing *lst, char *comment, ...);
static void asm_listing_add_comment(asm_listing *lst, char *comment, ...);
static void asm_listing_add_instruction(asm_listing *lst, asm_instruction *instr);


static struct asm_listing_ops ops = {
    .print = asm_listing_print,
    .set_next_label = asm_listing_set_next_label,
    .set_next_comment = asm_listing_set_next_comment,
    .add_comment = asm_listing_add_comment,
    .add_instruction = asm_listing_add_instruction,
};

asm_listing *new_asm_listing(mempool *mp) {
    asm_listing *l = mempool_alloc(mp, sizeof(asm_listing), "asm_listing");

    l->asm_lines = new_llist(mp);
    l->next_label = NULL;
    l->next_comment = NULL;
    l->ops = &ops;
    l->mempool = mp;

    return l;
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

static void asm_listing_print(asm_listing *lst, FILE *stream) {
    string *str = new_string();
    
    struct asm_instruction *inst;

    for_list(lst->asm_lines, asm_line, line) {
        inst = line->per_type.instruction;

        if (inst->label != NULL)
            fprintf(stream, "%s:\n", inst->label);
        
        asm_instruction_to_str(inst, str, true);
        fprintf(stream, "    %s\n", str->buffer);
        str->v->clear(str);

        // perhaps allow some space between functions?
        if (inst->operation == OC_RET)
            fprintf(stream, "\n\n");
    }

    str->v->free(str);
}

static void asm_listing_set_next_label(asm_listing *lst, char *label, ...) {
    char buffer[128];

    va_list vl;
    va_start(vl, label);
    vsnprintf(buffer, sizeof(buffer) - 1, label, vl);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(vl);

    lst->next_label = strdup(buffer);
}

static void asm_listing_set_next_comment(asm_listing *lst, char *comment, ...) {
    char buffer[128];

    va_list vl;
    va_start(vl, comment);
    vsnprintf(buffer, sizeof(buffer) - 1, comment, vl);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(vl);

    // keep aside for next instruction, same as label
    lst->next_comment = strdup(buffer);
}

static void asm_listing_add_comment(asm_listing *lst, char *comment, ...) {
    char buffer[128];

    va_list vl;
    va_start(vl, comment);
    vsnprintf(buffer, sizeof(buffer) - 1, comment, vl);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(vl);

    // add it as a standalone thing
    lst->next_comment = strdup(buffer);
    asm_listing_add_instruction(lst, new_asm_instruction(OC_NONE));
}

void asm_listing_add_instruction(asm_listing *lst, asm_instruction *instr) {
    instr->label = lst->next_label; // either null or not
    instr->comment = lst->next_comment; // null or not

    asm_line *line = mempool_alloc(lst->mempool, sizeof(asm_line), "asm_line");
    line->type = ALT_INSTRUCTION;
    line->per_type.instruction = instr;

    lst->next_label = NULL;
    lst->next_comment = NULL;
}

