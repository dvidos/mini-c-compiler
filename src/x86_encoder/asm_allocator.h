#include <stdbool.h>
#include "asm_listing.h"


// one storage unit, can be either on a register or on stack
typedef struct storage {
    bool is_gp_reg;
    bool is_stack_var;
    enum gp_reg gp_reg;
    int bp_offset; // negative for local vars, positive for args
    int size;
} storage;

struct storage_allocator_data {
    asm_listing *listing; // to grab stack space at runtime.
    int lowest_bp_offset; // equal to negative BP offset of last variable

    // function arguments and local variables.
    struct named_storage_slot {
        char *symbol_name;
        struct storage value;
    } *named_storage_arr;
    int named_storage_arr_len;

    // allocatable to temp_registers
    struct temp_storage_slot {
        int holder_reg; // non-zero means allocated.
        struct storage value; 
    } *temp_storage_arr;
    int temp_storage_arr_len;
};

struct storage_allocator_ops {
    void (*set_asm_listing)(asm_listing *lst); // once
    void (*reset)();
    void (*declare_local_symbol)(char *symbol, int size, int bp_offset);
    void (*generate_stack_info_comments)();

    bool (*get_named_storage)(char *symbol_name, storage *target); // false = not foudn
    void (*get_temp_reg_storage)(int reg_no, storage *target);
    void (*release_temp_reg_storage)(int reg_no);
};

extern struct storage_allocator_ops asm_allocator;
