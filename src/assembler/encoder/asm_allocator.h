#include <stdbool.h>
#include "asm_listing.h"


struct storage_allocator_ops;

typedef struct asm_allocator {
    void *private_data;
    struct storage_allocator_ops *ops;
} asm_allocator;

asm_allocator *new_asm_allocator(asm_listing *listing);


// one storage unit, can be either on a register or on stack
typedef struct storage {
    bool is_gp_reg;
    bool is_stack_var;
    enum gp_reg gp_reg;
    int bp_offset; // negative for local vars, positive for args
    int size;
} storage;

struct storage_allocator_ops {
    void (*reset)(asm_allocator *a);
    void (*declare_local_symbol)(asm_allocator *a, char *symbol, int size, int bp_offset);
    void (*generate_stack_info_comments)(asm_allocator *a);

    bool (*get_named_storage)(asm_allocator *a, char *symbol_name, storage *target); // false = not found
    void (*get_temp_reg_storage)(asm_allocator *a, int reg_no, storage *target);
    void (*release_temp_reg_storage)(asm_allocator *a, int reg_no);
};


