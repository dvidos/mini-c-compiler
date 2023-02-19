#pragma once
#include <stdbool.h>
#include "../lexer/token.h"

typedef struct expr_target expr_target;

typedef struct intermediate_representation_ops {
    void (*init)();

    void (*set_next_label)(char *fmt, ...);
    void (*add_str)(char *fmt, ...);
    void (*add_tac)(expr_target *target, char *fmt, ...);
    void (*add_comment)(char *fmt, ...);
    void (*jmp)(char *label_fmt, ...);
    // we would need SET, PUSH, POP, CALL, RET, SET, JUMPIF with operands...
    
    int (*reserve_data)(int bytes, void *init_data);
    int (*reserve_strz)(char *str);
    void (*add_symbol)(char *name, bool is_func, int offset);

    void (*dump_symbols)();
    void (*dump_code_segment)();
    void (*dump_data_segment)();

    void (*generate_assembly_listing)(char **buffer);
} intermediate_representation_ops;

extern intermediate_representation_ops ir;





// expression targets:
// - a numbered register value (e.g. a temp)
// - a named register (e.g. EAX) or BP offset
// - the address of a global data symbol
// - something pointed by any named or temp register
typedef struct expr_target {
    int numbered_register: 1;  // see number
    int return_register: 1;    // AX, EAX, RAX etc
    int named_symbol: 1;       // reference to data segment(s) (or code as well)
    int stack_location: 1;     // see offset
    int pointer_deref: 1;      // don't store at target, but where target points to.

    int specific_regsiter: 1;  // (0=AX, 1=BX, etc)
    int num_constant: 1;       // see value
    
    union { 
        int value;
        int offset;
        char *name;
    } u;
} expr_target;

expr_target *expr_target_temp_reg(int reg_no);
expr_target *expr_target_pointed_by_temp_reg(int reg_no);
expr_target *expr_target_return_register();
expr_target *expr_target_stack_location(int frame_offset);
expr_target *expr_target_named_symbol(char *symbol_name);
void expr_target_to_string(expr_target *target, char **str);
