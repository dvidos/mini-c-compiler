#pragma once
#include "../utils/buffer.h"
#include "symbol_table.h"
#include "reloc_list.h"



// in theory, compilation of each source file 
// would yield of a set of segments, a set of exported symbols, a set of relocations needed
// so we can give the "module" as a dependency to a x86_encoder.
struct module {
    buffer *text;
    buffer *data;
    buffer *bss;
    buffer *ro_data;

    // some symbols are exported, some are not
    // a symbol refers to a segment (section) and has an offset from it.
    symbol_table *symbols;

    // references in the code segment that need to be resolved at link time
    reloc_list *relocations;

    struct module_ops *ops;
};

typedef struct module module;

struct module_ops {
    // runtime manipulation
    void (*reset)(module *mod);
    void (*declare_data)(module *mod, char *symbol_name, u64 bytes, void *init_value);
    void (*print)(module *mod);
    
    // void (*encode_listing)(module *mod, listing *list);

    // // object file operations    
    // bool (*save_to_object_file)(char *filename);
    // bool (*load_from_object_file)(char *filename);

    // // linking of merged modules
    // bool (*append_module)(module *mod, module *source); // merges symbols as well
    // bool (*resolve_relocations)(u64 text_target_address, u64 data_target_address, u64 bss_base_address);
    // bool (*save_executable_file)(char *filename); // resolves symbols

    void (*free)();
};

module *new_module();

