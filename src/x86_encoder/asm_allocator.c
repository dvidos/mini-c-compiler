#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "asm_allocator.h"
#include "../options.h"


static struct storage_allocator_data data;

static void _set_asm_listing(asm_listing *lst); // once
static void _reset();
static void _declare_local_symbol(char *symbol, int size, int bp_offset);
static void _generate_stack_info_comments();
static bool _get_named_storage(char *symbol_name, storage *target); // false = not found
static void _get_temp_reg_storage(int reg_no, storage *target);
static void _release_temp_reg_storage(int reg_no);


struct storage_allocator_ops asm_allocator = {
    .set_asm_listing = _set_asm_listing,
    .reset = _reset,
    .declare_local_symbol = _declare_local_symbol,
    .generate_stack_info_comments = _generate_stack_info_comments,
    .get_named_storage = _get_named_storage,
    .get_temp_reg_storage = _get_temp_reg_storage,
    .release_temp_reg_storage = _release_temp_reg_storage,
};

static void _set_asm_listing(asm_listing *lst) {
    data.listing = lst;
}

static void _reset() {
    // clear all temp storage, put the first 5 gp registers
    if (data.named_storage_arr != NULL) {
        free(data.named_storage_arr);
        data.named_storage_arr = NULL;
        data.named_storage_arr_len = 0;
    }
    data.lowest_bp_offset = 0;

    if (data.temp_storage_arr != NULL) {
        free(data.named_storage_arr);
        data.temp_storage_arr = NULL;
        data.temp_storage_arr_len = 0;
    }

    // prepare the slots for the gp registers
    // AX is excluded to be allowed to work in any situation
    // SP and BP are excluded, as they ar eneeded for functions to work.
    // BX, CX, DX, SI, DI can be allocated.

    data.temp_storage_arr = malloc(5 * sizeof(struct temp_storage_slot));
    data.temp_storage_arr_len = 5;
    for (int i = 0; i < 5; i++)
        data.temp_storage_arr[i].value.is_gp_reg = 1;
    
    data.temp_storage_arr[0].value.gp_reg = REG_BX;
    data.temp_storage_arr[1].value.gp_reg = REG_CX;
    data.temp_storage_arr[2].value.gp_reg = REG_DX;
    data.temp_storage_arr[3].value.gp_reg = REG_SI;
    data.temp_storage_arr[4].value.gp_reg = REG_DI;
}

static void _declare_local_symbol(char *symbol, int size, int bp_offset) {
    // allocate more room
    data.named_storage_arr_len++;
    if (data.named_storage_arr_len == 1) {
        data.named_storage_arr = malloc(data.named_storage_arr_len * sizeof(struct named_storage_slot));
    } else {
        data.named_storage_arr = realloc(data.named_storage_arr, data.named_storage_arr_len * sizeof(struct named_storage_slot));
    }
    struct named_storage_slot *s = &data.named_storage_arr[data.named_storage_arr_len - 1];
    s->symbol_name = symbol;
    s->value.is_stack_var = true;
    s->value.bp_offset = bp_offset;
    s->value.size = size;

    // keep this for when we shall need more stack space
    if (bp_offset < data.lowest_bp_offset)
        data.lowest_bp_offset = bp_offset;
}

static void _generate_stack_info_comments() {
    char buffer[64];

    for (int i = 0; i < data.named_storage_arr_len; i++) {
        sprintf(buffer, "[%cBP%+3d] %s \"%s\", %d bytes",
            options.register_prefix,
            data.named_storage_arr[i].value.bp_offset,
            data.named_storage_arr[i].value.bp_offset < 0 ? "local var" : "argument",
            data.named_storage_arr[i].symbol_name,
            data.named_storage_arr[i].value.size);
        data.listing->ops->add_comment(data.listing, buffer, false);
    }
}

static bool _get_named_storage(char *symbol_name, storage *target) {
    for (int i = 0; i < data.named_storage_arr_len; i++) {
        if (strcmp(data.named_storage_arr[i].symbol_name, symbol_name) == 0) {
            memcpy(target, &data.named_storage_arr[i].value, sizeof(storage));
        }
    }
    return false; // not found
}

static void _get_temp_reg_storage(int reg_no, storage *target) {
    // first see if this register already holds something.
    for (int i = 0; i < data.temp_storage_arr_len; i++) {
        if (data.temp_storage_arr[i].holder_reg == reg_no) {
            memcpy(target, &data.temp_storage_arr[i].value, sizeof(storage));
            return;
        }
    }

    // find the first unowned slot to assign to this register
    for (int i = 0; i < data.temp_storage_arr_len; i++) {
        if (data.temp_storage_arr[i].holder_reg == 0) {
            data.temp_storage_arr[i].holder_reg = reg_no;
            memcpy(target, &data.temp_storage_arr[i].value, sizeof(storage));
            return;
        }
    }

    // take more space from stack, add one more slot, allocate it.
    // temp registers have the size of the architecture (32 or 64 bits)
    int size = options.pointer_size_bytes;
    data.lowest_bp_offset -= size;
    data.listing->ops->add_comment(data.listing, "grab some space for temp register", true);
    data.listing->ops->add_instr_reg_imm(data.listing, OC_SUB, REG_SP, size);
    data.temp_storage_arr_len += 1;
    data.temp_storage_arr = realloc(data.temp_storage_arr, data.temp_storage_arr_len * sizeof(struct temp_storage_slot));

    // set the slot data, copy to target
    struct temp_storage_slot *s = &data.temp_storage_arr[data.temp_storage_arr_len - 1];
    s->value.bp_offset = data.lowest_bp_offset;
    s->value.is_stack_var = true;
    s->value.size = size;
    s->holder_reg = reg_no;
    memcpy(target, &s->value, sizeof(storage));
}

static void _release_temp_reg_storage(int reg_no) {
    for (int i = 0; i < data.temp_storage_arr_len; i++) {
        if (data.temp_storage_arr[i].holder_reg == reg_no) {
            data.temp_storage_arr[i].holder_reg = 0;
            return;
        }
    }
}
