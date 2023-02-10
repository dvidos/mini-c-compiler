#pragma once
#include <stdbool.h>
#include "../lexer/token.h"

typedef struct intermediate_representation_ops {
    void (*init)();

    void (*set_next_label)(char *fmt, ...);
    void (*add_str)(char *fmt, ...);
    void (*add_comment)(char *fmt, ...);
    void (*jmp)(char *label_fmt, ...);
    
    int (*reserve_data)(int bytes, void *init_data);
    int (*reserve_strz)(char *str);
    void (*add_symbol)(char *name, bool is_func, int offset);

    void (*dump_symbols)();
    void (*dump_code_segment)();
    void (*dump_data_segment)();
} intermediate_representation_ops;

extern intermediate_representation_ops ir;

