#pragma once
#include "../utils/buffer.h"
#include "symbol_table.h"
#include "reloc_list.h"



// in theory, compilation of each source file 
// would yield of a set of segments, a set of exported symbols, a set of relocations needed
// so we can give the "obj_code" as a dependency to a x86_encoder.
struct obj_code {
    buffer *text_seg;
    buffer *data_seg;
    buffer *bss_seg;
    buffer *ro_data_seg;

    // some symbols are exported, some are not
    // a symbol refers to a segment (section) and has an offset from it.
    symbol_table *symbols;

    // references in the code segment that need to be resolved at link time
    reloc_list *relocations;

    struct module_ops *ops;
};

typedef struct obj_code obj_code;

struct module_ops {
    // runtime manipulation
    void (*reset)(obj_code *obj);
    void (*declare_data)(obj_code *obj, char *symbol_name, u64 bytes, void *init_value);
    void (*print)(obj_code *obj);
    
    // void (*encode_listing)(obj_code *obj, listing *list);

    // // object file operations    
    // bool (*save_to_object_file)(char *filename);
    // bool (*load_from_object_file)(char *filename);

    // // linking of merged modules
    // bool (*append_module)(obj_code *obj, obj_code *source); // merges symbols as well
    // bool (*resolve_relocations)(u64 text_target_address, u64 data_target_address, u64 bss_base_address);
    // bool (*save_executable_file)(char *filename); // resolves symbols

    void (*free)();
};

obj_code *new_obj_code_module();

