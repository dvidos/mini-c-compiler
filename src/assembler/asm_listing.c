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
static void asm_listing_select_section(asm_listing *lst, const char *name, ...);
static void asm_listing_declare_global(asm_listing *lst, const char *name, ...);
static void asm_listing_declare_extern(asm_listing *lst, const char *name, ...);
static void asm_listing_set_next_label(asm_listing *lst, const char *label, ...);
static void asm_listing_set_next_comment(asm_listing *lst, char *comment, ...);
static void asm_listing_add_comment(asm_listing *lst, char *comment, ...);
static void asm_listing_add_line(asm_listing *lst, asm_line *line);


static struct asm_listing_ops ops = {
    .print = asm_listing_print,
    .set_next_label = asm_listing_set_next_label,
    .select_section = asm_listing_select_section,
    .declare_global = asm_listing_declare_global,
    .declare_extern = asm_listing_declare_extern,
    .set_next_comment = asm_listing_set_next_comment,
    .add_comment = asm_listing_add_comment,
    .add_line = asm_listing_add_line,
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
    for_list(lst->lines, asm_line, line) {
        str *s = asm_line_to_str(mp, line);
        fprintf(stream, "%s\n", str_charptr(s));

        // perhaps allow some space between functions?
        if (line->type == ALT_INSTRUCTION && line->per_type.instruction->operation == OC_RET)
            fprintf(stream, "\n\n");
    }
    mempool_release(mp);
}

static void asm_listing_select_section(asm_listing *lst, const char *name, ...) {
    va_list vl;
    va_start(vl, name);
    str *section = new_strv(lst->mempool, name, vl);
    va_end(vl);
    list_add(lst->lines, new_asm_line_directive_section(lst->mempool, section));
}

static void asm_listing_declare_global(asm_listing *lst, const char *name, ...) {
    va_list vl;
    va_start(vl, name);
    str *symbol = new_strv(lst->mempool, name, vl);
    va_end(vl);
    list_add(lst->lines, new_asm_line_directive_global(lst->mempool, symbol));
}

static void asm_listing_declare_extern(asm_listing *lst, const char *name, ...) {
    va_list vl;
    va_start(vl, name);
    str *symbol = new_strv(lst->mempool, name, vl);
    va_end(vl);
    list_add(lst->lines, new_asm_line_directive_extern(lst->mempool, symbol));
}

static void asm_listing_set_next_label(asm_listing *lst, const char *label, ...) {
    va_list vl;
    va_start(vl, label);
    lst->next_label = new_strv(lst->mempool, label, vl);
    va_end(vl);
}

static void asm_listing_set_next_comment(asm_listing *lst, char *comment, ...) {
    va_list vl;
    va_start(vl, comment);
    lst->next_comment = new_strv(lst->mempool, comment, vl);
    va_end(vl);
}

static void asm_listing_add_comment(asm_listing *lst, char *comment, ...) {
    va_list vl;
    va_start(vl, comment);
    lst->next_comment = new_strv(lst->mempool, comment, vl);
    va_end(vl);
    // add it as a standalone line
    asm_listing_add_line(lst, new_asm_line_instruction(lst->mempool, OC_NONE));
}

static void asm_listing_add_line(asm_listing *lst, asm_line *line) {
    line->label = lst->next_label;     // null or not
    line->comment = lst->next_comment; // null or not
    list_add(lst->lines, line);
    lst->next_label = NULL;
    lst->next_comment = NULL;
}

