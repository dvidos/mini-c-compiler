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


struct obj_code_ops ops = {
    .set_name = _set_name,
    .reset = _reset,
    .declare_data = _declare_data,
    .print = _print,
    .save_object_file = _save_object_file,
    .free = _free,
};

obj_code *new_obj_code_module() {
    obj_code *m = malloc(sizeof(obj_code));

    m->text_seg = new_buffer();
    m->data_seg = new_buffer();
    m->bss_seg = new_buffer();
    m->symbols = new_symbol_table();
    m->relocations = new_reloc_list();
    m->ops = &ops;

    return m;
}

static void _set_name(obj_code *obj, char *name) {
    obj->name = name == NULL ? NULL : strdup(name);
}

static void _declare_data(obj_code *obj, char *symbol_name, u64 bytes, void *init_value) {
    if (init_value == NULL) {
        u64 address = obj->bss_seg->length;
        obj->bss_seg->add_zeros(obj->bss_seg, bytes);
        obj->symbols->add(obj->symbols, symbol_name, address, SB_BSS);
    } else {
        u64 address = obj->bss_seg->length;
        obj->data_seg->add_mem(obj->data_seg, init_value, bytes);
        obj->symbols->add(obj->symbols, symbol_name, address, SB_DATA);
    }
}

static void _reset(obj_code *obj) {
    obj->text_seg->clear(obj->text_seg);
    obj->data_seg->clear(obj->data_seg);
    obj->bss_seg->clear(obj->bss_seg);
    obj->relocations->clear(obj->relocations);
    obj->symbols->clear(obj->symbols);
}

static void _print(obj_code *obj) {
    if (obj->text_seg->length > 0) {
        printf("Code (%d bytes)\n", obj->text_seg->length);
        print_16_hex(obj->text_seg->buffer, obj->text_seg->length, 2);
    }
    if (obj->relocations->length > 0) {
        printf("Relocations (%d entries)\n", obj->relocations->length);
        obj->relocations->print(obj->relocations);
    }
    if (obj->data_seg->length > 0) {
        printf("Data (%d bytes):\n", obj->data_seg->length);
        print_16_hex(obj->data_seg->buffer, obj->data_seg->length, 2);
    }
    if (obj->bss_seg->length > 0) {
        printf("Bss (%d bytes):\n", obj->bss_seg->length);
        print_16_hex(obj->bss_seg->buffer, obj->bss_seg->length, 2);
    }
    if (obj->relocations->length > 0) {
        printf("Symbols (%d entries)\n", obj->relocations->length);
        obj->symbols->print(obj->symbols);
    }
}

static bool _save_object_file(obj_code *obj, FILE *f) {
    return false;
}

static void _free(obj_code *obj) {
    obj->text_seg->free(obj->text_seg);
    obj->data_seg->free(obj->data_seg);
    obj->bss_seg->free(obj->bss_seg);
    obj->relocations->free(obj->relocations);
    obj->symbols->free(obj->symbols);
    free(obj);
}
