#pragma once
#include <stdbool.h>
#include "../lexer/token.h"



// usage from code that is generating intermediate
void init_intermediate_representation();

int ir_get_strz_address(char *value, token *token);
void ir_reserve_data_area(char *name, int size, bool initialized, void *initial_data);

void ir_set_next_label(char *fmt, ...);
void ir_add_str(char *fmt, ...);
void ir_add_comment(char *fmt, ...);
void ir_jmp(char *label_fmt, ...);



// usage to dump what is generated into output
void ir_dump_data_segment(bool initialized);
void ir_dump_code_segment();

