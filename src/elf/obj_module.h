#pragma once
#include <stdio.h>
#include "../utils/data_types.h"
#include "../utils/data_structs.h"
#include "elf64_contents.h"
#include "obj_section.h"

typedef struct obj_module     obj_module;
typedef struct obj_symbol     obj_symbol;
typedef struct obj_relocation obj_relocation;


struct obj_module {
    str *name;        // e.g. "mcc.c"
    llist *sections;  // item type is obj_section

    struct obj_module_ops {
        void (*print)(obj_module *m, FILE *f);
        void (*append)(obj_module *m, struct obj_module *source);
        obj_section *(*get_section_by_name)(obj_module *m, str *name);
        obj_symbol *(*find_symbol)(obj_module *m, str *name, bool exported);
        elf64_contents *(*pack_object_file)(obj_module *m, mempool *mp);
        elf64_contents *(*pack_executable_file)(obj_module *m, mempool *mp);
    } *ops;

    mempool *mempool;
};


obj_module *new_obj_module(mempool *mp);
obj_module *new_obj_module_from_elf64_contents(elf64_contents *contents, mempool *mp);

