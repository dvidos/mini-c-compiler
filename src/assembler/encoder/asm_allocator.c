#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "asm_allocator.h"
#include "../../run_info.h"


struct asm_allocator_data {
    asm_listing *listing; // to grab stack space at runtime.
    int lowest_bp_offset; // equal to negative BP offset of last variable

    // function arguments and local variables.
    struct named_storage_slot {
        const char *symbol_name;
        struct storage value;
    } *named_storage_arr;
    int named_storage_arr_len;

    // allocatable to temp_registers
    struct temp_storage_slot {
        int holder_reg; // non-zero means allocated.
        struct storage value; 
    } *temp_storage_arr;
    int temp_storage_arr_len;

    mempool *mempool;
};


static void _reset(asm_allocator *a);
static void _declare_local_symbol(asm_allocator *a, const char *symbol, int size, int bp_offset);
static void _generate_stack_info_comments(asm_allocator *a);
static bool _get_named_storage(asm_allocator *a, const char *symbol_name, storage *target); // false = not found
static void _get_temp_reg_storage(asm_allocator *a, int temp_reg_no, storage *target, bool *allocated);
static bool _is_treg_a_gp_reg(asm_allocator *a, int reg_no);
static bool _is_treg_a_stack_var(asm_allocator *a, int reg_no);
static void _release_temp_reg_storage(asm_allocator *a, int temp_reg_no);
static void _storage_to_str(asm_allocator *a, storage *stor, str *s);


static struct storage_allocator_ops ops = {
    .reset = _reset,
    .declare_local_symbol = _declare_local_symbol,
    .generate_stack_info_comments = _generate_stack_info_comments,
    .get_named_storage = _get_named_storage,
    .get_temp_reg_storage = _get_temp_reg_storage,
    .is_treg_a_gp_reg = _is_treg_a_gp_reg,
    .is_treg_a_stack_var = _is_treg_a_stack_var,
    .release_temp_reg_storage = _release_temp_reg_storage,
    .storage_to_str = _storage_to_str
};

asm_allocator *new_asm_allocator(mempool *mp, asm_listing *listing) {

    asm_allocator *a = mpalloc(mp, asm_allocator);
    struct asm_allocator_data *data = mpalloc(mp, struct asm_allocator_data);
    memset(data, 0, sizeof(struct asm_allocator_data));

    data->listing = listing;
    data->mempool = mp;

    a->private_data = data;
    a->ops = &ops;
    return a;
}


static void _reset(asm_allocator *a) {
    struct asm_allocator_data *data = (struct asm_allocator_data *)a->private_data;
    
    // clear all temp storage, put the first 5 gp registers
    if (data->named_storage_arr != NULL) {
        free(data->named_storage_arr);
        data->named_storage_arr = NULL;
        data->named_storage_arr_len = 0;
    }
    data->lowest_bp_offset = 0;

    if (data->temp_storage_arr != NULL) {
        free(data->named_storage_arr);
        data->temp_storage_arr = NULL;
        data->temp_storage_arr_len = 0;
    }

    // prepare the slots for the gp registers
    // AX is excluded to be allowed to work in any situation
    // SP and BP are excluded, as they ar eneeded for functions to work.
    // BX, CX, DX, SI, DI can be allocated.

    data->temp_storage_arr_len = 5;
    data->temp_storage_arr = malloc(data->temp_storage_arr_len * sizeof(struct temp_storage_slot));
    memset(data->temp_storage_arr, 0, data->temp_storage_arr_len * sizeof(struct temp_storage_slot));
    for (int i = 0; i < 5; i++)
        data->temp_storage_arr[i].value.is_gp_reg = 1;
    
    data->temp_storage_arr[0].value.gp_reg = REG_BX;
    data->temp_storage_arr[1].value.gp_reg = REG_CX;
    data->temp_storage_arr[2].value.gp_reg = REG_DX;
    data->temp_storage_arr[3].value.gp_reg = REG_SI;
    data->temp_storage_arr[4].value.gp_reg = REG_DI;

    // printf("Prepared storage allocation table:\n"); for (int j=0;j<data->temp_storage_arr_len;j++) printf("  #%d  owner=%d, is_gp=%d, gp=%d, is_bp_off=%d, bp_off=%d\n", j, data->temp_storage_arr[j].holder_reg, data->temp_storage_arr[j].value.is_gp_reg, data->temp_storage_arr[j].value.gp_reg, data->temp_storage_arr[j].value.is_stack_var, data->temp_storage_arr[j].value.bp_offset);
}

static void _declare_local_symbol(asm_allocator *a, const char *symbol, int size, int bp_offset) {
    struct asm_allocator_data *data = (struct asm_allocator_data *)a->private_data;
    
    // allocate more room
    data->named_storage_arr_len++;
    if (data->named_storage_arr_len == 1) {
        data->named_storage_arr = malloc(data->named_storage_arr_len * sizeof(struct named_storage_slot));
    } else {
        data->named_storage_arr = realloc(data->named_storage_arr, data->named_storage_arr_len * sizeof(struct named_storage_slot));
    }
    struct named_storage_slot *s = &data->named_storage_arr[data->named_storage_arr_len - 1];
    s->symbol_name = symbol;
    s->value.is_stack_var = true;
    s->value.bp_offset = bp_offset;
    s->value.size = size;

    // keep this for when we shall need more stack space
    if (bp_offset < data->lowest_bp_offset)
        data->lowest_bp_offset = bp_offset;
}

static void _generate_stack_info_comments(asm_allocator *a) {
    struct asm_allocator_data *data = (struct asm_allocator_data *)a->private_data;
    
    char buffer[64];

    for (int i = 0; i < data->named_storage_arr_len; i++) {
        data->listing->ops->add_comment(data->listing, "[%cBP%+3d] %s \"%s\", %d bytes",
            run_info->options->register_prefix,
            data->named_storage_arr[i].value.bp_offset,
            data->named_storage_arr[i].value.bp_offset < 0 ? "local var" : "argument",
            data->named_storage_arr[i].symbol_name,
            data->named_storage_arr[i].value.size);
    }
}

static bool _get_named_storage(asm_allocator *a, const char *symbol_name, storage *target) {
    struct asm_allocator_data *data = (struct asm_allocator_data *)a->private_data;
    
    for (int i = 0; i < data->named_storage_arr_len; i++) {
        if (strcmp(data->named_storage_arr[i].symbol_name, symbol_name) == 0) {
            memcpy(target, &data->named_storage_arr[i].value, sizeof(storage));
            return true;
        }
    }
    return false; // not found
}

static void _get_temp_reg_storage(asm_allocator *a, int temp_reg_no, storage *target, bool *allocated) {
    struct asm_allocator_data *data = (struct asm_allocator_data *)a->private_data;
    *allocated = false;
    
    // first see if this register already holds something.
    for (int i = 0; i < data->temp_storage_arr_len; i++) {
        if (data->temp_storage_arr[i].holder_reg == temp_reg_no) {
            memcpy(target, &data->temp_storage_arr[i].value, sizeof(storage));
            return;
        }
    }

    // if we got here, we need to allocate a storage
    *allocated = true;

    // find the first unowned slot to assign to this register
    for (int i = 0; i < data->temp_storage_arr_len; i++) {
        if (data->temp_storage_arr[i].holder_reg == 0) {
            data->temp_storage_arr[i].holder_reg = temp_reg_no;
            memcpy(target, &data->temp_storage_arr[i].value, sizeof(storage));
            return;
        }
    }

    // take more space from stack, add one more slot, allocate it.
    // temp registers have the size of the architecture (32 or 64 bits)
    int size = run_info->options->pointer_size_bytes;
    data->lowest_bp_offset -= size;
    data->listing->ops->set_next_comment(data->listing, "grab some space for temp register");

    // we need assembler here... to allocate stack for temp register...
    data->listing->ops->add_instruction(data->listing, new_asm_instruction_for_reserving_stack_space(size));
    data->temp_storage_arr_len += 1;
    data->temp_storage_arr = realloc(data->temp_storage_arr, data->temp_storage_arr_len * sizeof(struct temp_storage_slot));

    // set the slot data, copy to target
    struct temp_storage_slot *s = &data->temp_storage_arr[data->temp_storage_arr_len - 1];
    s->value.bp_offset = data->lowest_bp_offset;
    s->value.is_stack_var = true;
    s->value.size = size;
    s->holder_reg = temp_reg_no;
    memcpy(target, &s->value, sizeof(storage));
}

static bool _is_treg_a_gp_reg(asm_allocator *a, int reg_no) {
    struct asm_allocator_data *data = (struct asm_allocator_data *)a->private_data;

    for (int i = 0; i < data->temp_storage_arr_len; i++) {
        if (data->temp_storage_arr[i].holder_reg == reg_no) {
            return data->temp_storage_arr[i].value.is_gp_reg;
        }
    }
    return false;
}

static bool _is_treg_a_stack_var(asm_allocator *a, int reg_no) {
    struct asm_allocator_data *data = (struct asm_allocator_data *)a->private_data;
    
    for (int i = 0; i < data->temp_storage_arr_len; i++) {
        if (data->temp_storage_arr[i].holder_reg == reg_no) {
            return data->temp_storage_arr[i].value.is_stack_var;
        }
    }
    return false;
}

static void _release_temp_reg_storage(asm_allocator *a, int temp_reg_no) {
    struct asm_allocator_data *data = (struct asm_allocator_data *)a->private_data;
    
    for (int i = 0; i < data->temp_storage_arr_len; i++) {
        if (data->temp_storage_arr[i].holder_reg == temp_reg_no) {
            data->temp_storage_arr[i].holder_reg = 0;
            return;
        }
    }
}

static void _storage_to_str(asm_allocator *a, storage *stor, str *s) {
    if (stor->is_gp_reg) {
        str_cats(s, gp_reg_name(stor->gp_reg));
    } else if (stor->is_stack_var) {
        str_catf(s, "[BP%+d] (%d bytes)", stor->bp_offset, stor->size);
    }
}
