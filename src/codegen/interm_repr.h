#pragma once
#include <stdbool.h>



// usage from code that is generating intermediate
void init_intermediate_representation();
void reserve_data_area(char *name, int size, bool initialized, void *initial_data);
void add_jump_operation(char *label, bool conditional, char *target_label);
void add_output_operation(char *label, int op, int address1, int address2, int address3);

int get_next_reg_num();
int get_next_if_num();
int get_curr_while_num();
void push_while();
void pop_while();


void ir_set_next_label(char *fmt, ...);
void ir_add_str(char *fmt, ...);
void ir_jmp(char *label_fmt, ...);




// usage to dump what is generated into output
void ir_dump_data_segment(bool initialized);
void ir_dump_code_segment();

