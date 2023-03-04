#include <stdlib.h>
#include "buffer.h"
#include "symbol_table.h"
#include "reloc_list.h"
#include "module.h"


static void _free(module *mod);
static void _reset(module *mod);
static void _declare_data(module *mod, char *symbol_name, u64 bytes, void *init_value);

struct module_ops ops = {
    .reset = _reset,
    .declare_data = _declare_data,
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

static void _free(module *mod) {
    mod->text->free(mod->text);
    mod->data->free(mod->data);
    mod->bss->free(mod->bss);
    mod->relocations->free(mod->relocations);
    mod->symbols->free(mod->symbols);
    free(mod);
}