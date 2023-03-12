#include <stdlib.h>
#include <stdio.h>
#include "../utils/buffer.h"
#include "symbol_table.h"
#include "reloc_list.h"
#include "module.h"
#include "../utils.h"


static void _free(module *mod);
static void _reset(module *mod);
static void _declare_data(module *mod, char *symbol_name, u64 bytes, void *init_value);
static void _print(module *mod);

struct module_ops ops = {
    .reset = _reset,
    .declare_data = _declare_data,
    .print = _print,
    .free = _free,
};

module *new_module() {
    module *m = malloc(sizeof(module));

    m->text = new_buffer();
    m->data = new_buffer();
    m->bss = new_buffer();
    m->symbols = new_symbol_table();
    m->relocations = new_reloc_list();
    m->ops = &ops;

    return m;
}

static void _declare_data(module *mod, char *symbol_name, u64 bytes, void *init_value) {
    if (init_value == NULL) {
        u64 offset = mod->bss->length;
        mod->bss->add_zeros(mod->bss, bytes);
        mod->symbols->add(mod->symbols, symbol_name, offset, SB_ZERO_DATA);
    } else {
        u64 offset = mod->bss->length;
        mod->data->add_mem(mod->data, init_value, bytes);
        mod->symbols->add(mod->symbols, symbol_name, offset, SB_DATA);
    }
}

static void _reset(module *mod) {
    mod->text->clear(mod->text);
    mod->data->clear(mod->data);
    mod->bss->clear(mod->bss);
    mod->relocations->clear(mod->relocations);
    mod->symbols->clear(mod->symbols);
}

static void _print(module *mod) {
    if (mod->text->length > 0) {
        printf("Code (%d bytes)\n", mod->text->length);
        print_16_hex(mod->text->data, mod->text->length, 2);
    }
    if (mod->relocations->length > 0) {
        printf("Relocations (%d entries)\n", mod->relocations->length);
        mod->relocations->print(mod->relocations);
    }
    if (mod->data->length > 0) {
        printf("Data (%d bytes):\n", mod->data->length);
        print_16_hex(mod->data->data, mod->data->length, 2);
    }
    if (mod->bss->length > 0) {
        printf("Bss (%d bytes):\n", mod->bss->length);
        print_16_hex(mod->bss->data, mod->bss->length, 2);
    }
    if (mod->relocations->length > 0) {
        printf("Symbols (%d entries)\n", mod->relocations->length);
        mod->symbols->print(mod->symbols);
    }
}

static void _free(module *mod) {
    mod->text->free(mod->text);
    mod->data->free(mod->data);
    mod->bss->free(mod->bss);
    mod->relocations->free(mod->relocations);
    mod->symbols->free(mod->symbols);
    free(mod);
}