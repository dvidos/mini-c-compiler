#pragma once
#include <stdbool.h>



// usage from code that is generating intermediate
void init_intermediate_representation();
void reserve_data_area(char *name, int size, bool initialized);
void add_jump_operation(char *label, bool conditional, char *target_label);
void add_output_operation(char *label, int op, int address1, int address2, int address3);


// usage to dump what is generated into output
void ir_dump_data_segment(bool initialized);
void ir_dump_code_segment();

