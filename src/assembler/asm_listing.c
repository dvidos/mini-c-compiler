#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../utils/all.h"
#include "asm_line.h"
#include "asm_listing.h"


static void asm_listing_print(asm_listing *lst, FILE *stream);
static void asm_listing_ensure_capacity(asm_listing *lst, int extra);
static void asm_listing_set_next_label(asm_listing *lst, const char *label, ...);
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
    asm_listing *l = mpalloc(mp, asm_listing);

    l->lines = new_list(mp);
    l->next_label = NULL;
    l->next_comment = NULL;
    l->ops = &ops;
    l->mempool = mp;

    return l;
}

static void asm_listing_print(asm_listing *lst, FILE *stream) {
    mempool *mp = new_mempool();
    str *s = new_str(mp, NULL);
    
    struct asm_instruction *inst;

    for_list(lst->lines, asm_line, line) {
        if (line->label != NULL)
            fprintf(stream, "%s:\n", str_charptr(line->label));
        
        inst = line->per_type.instruction;

        asm_instruction_to_str(inst, s, true);
        fprintf(stream, "    %s\n", str_charptr(s));
        str_clear(s);

        // perhaps allow some space between functions?
        if (inst->operation == OC_RET)
            fprintf(stream, "\n\n");
    }

    mempool_release(mp);
}

static void asm_listing_set_next_label(asm_listing *lst, const char *label, ...) {
    char buffer[128];

    va_list vl;
    va_start(vl, label);
    vsnprintf(buffer, sizeof(buffer) - 1, label, vl);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(vl);

    lst->next_label = new_str(lst->mempool, buffer);
}

static void asm_listing_set_next_comment(asm_listing *lst, char *comment, ...) {
    char buffer[128];

    va_list vl;
    va_start(vl, comment);
    vsnprintf(buffer, sizeof(buffer) - 1, comment, vl);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(vl);

    // keep aside for next instruction, same as label
    lst->next_comment = new_str(lst->mempool, buffer);
}

static void asm_listing_add_comment(asm_listing *lst, char *comment, ...) {
    char buffer[128];

    va_list vl;
    va_start(vl, comment);
    vsnprintf(buffer, sizeof(buffer) - 1, comment, vl);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(vl);

    // add it as a standalone thing
    lst->next_comment = new_str(lst->mempool, buffer);
    asm_listing_add_instruction(lst, new_asm_instruction(OC_NONE));
}

void asm_listing_add_instruction(asm_listing *lst, asm_instruction *instr) {

    asm_line *line = mpalloc(lst->mempool, asm_line);
    line->label = lst->next_label; // either null or not
    line->comment = lst->next_comment; // null or not
    line->type = ALT_INSTRUCTION;
    line->per_type.instruction = instr;

    list_add(lst->lines, line);
    lst->next_label = NULL;
    lst->next_comment = NULL;
}

