#include <string.h>
#include "elf_format.h"
#include "obj_section.h"



static void obj_section_print(obj_section *s, bool show_details, FILE *f);
static void obj_section_append(obj_section *s, obj_section *other, size_t rounding_value);
static void obj_section_change_address(obj_section *s, long delta);
static void obj_section_rebase(obj_section *s, long delta);
static obj_symbol *obj_section_find_symbol(obj_section *s, str *name, bool exported);

static void obj_symbol_print(obj_symbol *s, int num, FILE *f);

static void obj_relocation_print(obj_relocation *r, FILE *f);

static struct obj_section_ops section_ops = {
    .print = obj_section_print,
    .append = obj_section_append,
    .change_address = obj_section_change_address,
    .rebase = obj_section_rebase,
    .find_symbol = obj_section_find_symbol,
};

static struct obj_symbol_ops symbol_ops = {
    .print = obj_symbol_print
};

static struct obj_relocation_ops relocation_ops = {
    .print = obj_relocation_print
};

obj_section *new_obj_section(mempool *mp) {
    obj_section *s = mempool_alloc(mp, sizeof(obj_section), "obj_section");
    s->name = new_str(mp, "");
    s->contents = new_bin(mp);
    s->relocations = new_llist(mp);
    s->symbols = new_llist(mp);
    s->ops = &section_ops;
    return s;
}

static void obj_symbol_print(obj_symbol *s, int num, FILE *f) {
    if (s == NULL) {
        fprintf(f, "    Symbols\n");
        fprintf(f, "      Num    Value     Size  Scope   Name\n");
        //                Num    Value     Size  Scope   Name
        //                123 12345678 12345678  GLOBAL  123456789...
    } else {
        fprintf(f, "      %3d %08lx %8lu  %-6s  %s\n",
            num,
            s->value,
            s->size,
            s->global ? "GLOBAL" : "LOCAL",
            str_charptr(s->name));
    }
}

static void obj_relocation_print(obj_relocation *r, FILE *f) {
    if (r == NULL) { 
        fprintf(f, "    Relocations\n");
        fprintf(f, "        Offset  Type  Addendum  Symbol\n");
        //         "        Offset  Type  Addendum  Symbol
        //         "      12345678  1234  12345678  12345...
    } else {
        fprintf(f, "      %08lx  %4d  %+8ld  %s\n",
            r->offset,
            r->type,
            r->addendum,
            str_charptr(r->symbol_name));
    }
}

static void obj_section_print(obj_section *s, bool show_details, FILE *f) {
    fprintf(f, "  Section %-10s %ld bytes at 0x%lx\n", str_charptr(s->name), bin_len(s->contents), s->address);

    if (show_details && bin_len(s->contents) > 0) {
        if (s->flags.init_to_zero) {
            // in zero init, all the buffer's contents would be reset to zero.
            fprintf(f, "    Contents is %lu bytes, initialized to zero\n", bin_len(s->contents));
        } else {
            size_t bytes = bin_len(s->contents) > 64 ? 64 : bin_len(s->contents);
            fprintf(f, "    Contents (%lu / %lu total bytes)\n", bytes, bin_len(s->contents));
            bin_print_hex(s->contents, 6, 0, bytes, f);
        }
    }

    if (show_details && llist_length(s->symbols) > 0) {
        int num = 0;
        obj_symbol_print(NULL, 0, f);
        for_list(s->symbols, obj_symbol, sym)
            obj_symbol_print(sym, num++, f);
    }

    if (show_details && llist_length(s->relocations)) {
        obj_relocation_print(NULL, f);
        for_list(s->relocations, obj_relocation, r)
            obj_relocation_print(r, f);
    }
}

static void obj_section_append(obj_section *s, obj_section *other, size_t rounding_value) {
    // let's up this one to the size of a long (64 bits, or 8 bytes)
    size_t size = bin_len(s->contents);
    if (rounding_value > 1) {
        size = ((size + (rounding_value - 1)) / rounding_value) * rounding_value;
        bin_pad(s->contents, 0, size);
    }

    // size will now be the new base of relocations and symbols.
    bin_cat(s->contents, other->contents);
    other->ops->rebase(other, (long)size);
    llist_add_all(s->symbols, other->symbols);
    llist_add_all(s->relocations, other->relocations);
}

static void obj_section_change_address(obj_section *s, long delta) {
    s->address += delta;
    for_list(s->symbols, obj_symbol, sym) {
        sym->value += delta;
    }
}

static void obj_section_rebase(obj_section *s, long delta) {
    s->address += delta;
    for_list(s->symbols, obj_symbol, sym) {
        sym->value += delta;
    }
    for_list(s->relocations, obj_relocation, rel) {
        rel->offset += delta;
    }
}

static obj_symbol *obj_section_find_symbol(obj_section *s, str *name, bool exported) {
    for_list (s->symbols, obj_symbol, sym) {
        if (str_cmp(sym->name, name) != 0)
            continue;
        
        // do we need it to be exported?
        if (exported && !sym->global)
            continue;
        
        return sym;
    }
    return NULL;
}


