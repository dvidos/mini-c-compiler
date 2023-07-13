#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "symbol_table.h"
#include "reloc_list.h"
#include "obj_code.h"
#include "../utils.h"


static void _set_name(obj_code *obj, char *name);
static void _reset(obj_code *obj);
static void _declare_data(obj_code *obj, char *symbol_name, u64 bytes, void *init_value);
static void _print(obj_code *obj);
static bool _save_object_file(obj_code *obj, FILE *f);


struct obj_code_vtable ops = {
    .set_name = _set_name,
    .reset = _reset,
    .declare_data = _declare_data,
    .print = _print,
    .save_object_file = _save_object_file,
};

obj_code *new_obj_code(mempool *mp) {
    obj_code *m = mpalloc(mp, obj_code);

    m->text = new_section(mp);
    m->text->v->set_name(m->text, ".text");

    m->data = new_section(mp);
    m->data->v->set_name(m->data, ".data");
    
    m->bss = new_section(mp);
    m->bss->v->set_name(m->bss, ".bss");
    
    m->rodata = new_section(mp);
    m->rodata->v->set_name(m->rodata, ".rodata");

    m->vt = &ops;
    return m;
}

static void _set_name(obj_code *obj, char *name) {
    obj->name = name == NULL ? NULL : strdup(name);
}

static void _declare_data(obj_code *obj, char *symbol_name, u64 bytes, void *init_value) {
    section *s = (init_value == NULL) ? obj->bss : obj->data;

    u64 address = bin_len(s->contents);
    if (init_value == NULL)
        bin_add_zeros(s->contents, bytes);
    else
        bin_add_mem(s->contents, init_value, bytes);
    s->symbols->add(s->symbols, symbol_name, address, bytes, ST_OBJECT, true);
}

static void _reset(obj_code *obj) {
    // obj->text->reset(obj->text);
    // obj->data->reset(obj->data);
    // obj->bss->reset(obj->bss);
    // obj->rodata->reset(obj->rodata);
}

static void _print(obj_code *obj) {
    if (bin_len(obj->text->contents) > 0 || obj->text->symbols->length > 0 || obj->text->relocations->length > 0)
        obj->text->v->print(obj->text);
    
    if (bin_len(obj->data->contents) > 0 || obj->data->symbols->length > 0 || obj->data->relocations->length > 0)
        obj->data->v->print(obj->data);

    if (bin_len(obj->bss->contents) > 0 || obj->bss->symbols->length > 0 || obj->bss->relocations->length > 0)
        obj->bss->v->print(obj->bss);

    if (bin_len(obj->rodata->contents) > 0 || obj->rodata->symbols->length > 0 || obj->rodata->relocations->length > 0)
        obj->rodata->v->print(obj->rodata);
}

static bool _save_object_file(obj_code *obj, FILE *f) {
    return false;
}
