#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../utils/buffer.h"
#include "symbol_table.h"
#include "reloc_list.h"
#include "obj_code.h"
#include "../utils.h"


static void _set_name(obj_code *obj, char *name);
static void _reset(obj_code *obj);
static void _declare_data(obj_code *obj, char *symbol_name, u64 bytes, void *init_value);
static void _print(obj_code *obj);
static bool _save_object_file(obj_code *obj, FILE *f);
static void _free(obj_code *obj);


struct obj_code_vtable ops = {
    .set_name = _set_name,
    .reset = _reset,
    .declare_data = _declare_data,
    .print = _print,
    .save_object_file = _save_object_file,
    .free = _free,
};

obj_code *new_obj_code() {
    obj_code *m = malloc(sizeof(obj_code));

    m->text = new_section();
    m->text->v->set_name(m->text, ".text");

    m->data = new_section();
    m->data->v->set_name(m->data, ".data");
    
    m->bss = new_section();
    m->bss->v->set_name(m->bss, ".bss");
    
    m->rodata = new_section();
    m->rodata->v->set_name(m->rodata, ".rodata");

    m->vt = &ops;
    return m;
}

static void _set_name(obj_code *obj, char *name) {
    obj->name = name == NULL ? NULL : strdup(name);
}

static void _declare_data(obj_code *obj, char *symbol_name, u64 bytes, void *init_value) {
    if (init_value == NULL) {
        u64 address = obj->bss->contents->length;
        obj->bss->contents->add_zeros(obj->bss->contents, bytes);
        obj->bss->symbols->add(obj->bss->symbols, symbol_name, address, SB_BSS);
    } else {
        u64 address = obj->bss->contents->length;
        obj->data->contents->add_mem(obj->data->contents, init_value, bytes);
        obj->data->symbols->add(obj->data->symbols, symbol_name, address, SB_DATA);
    }
}

static void _reset(obj_code *obj) {
    // obj->text->reset(obj->text);
    // obj->data->reset(obj->data);
    // obj->bss->reset(obj->bss);
    // obj->rodata->reset(obj->rodata);
}

static void _print(obj_code *obj) {
    obj->text->v->print(obj->text);
    obj->data->v->print(obj->data);
    obj->bss->v->print(obj->bss);
    obj->rodata->v->print(obj->rodata);
}

static bool _save_object_file(obj_code *obj, FILE *f) {
    return false;
}

static void _free(obj_code *obj) {
    if (obj->name != NULL)
        free(obj->name);
    obj->text->v->free(obj->text);
    obj->data->v->free(obj->data);
    obj->bss->v->free(obj->bss);
    obj->rodata->v->free(obj->rodata);
    free(obj);
}
