#pragma once
#include "obj_code.h"


// arrange code, ro_data, data, bss, etc, align in 4k pages, write 
// relocate as needed, resolve references, write elf file
void x86_link(
    obj_code **modules_arr,
    int modules_length,
    u64 base_address,
    char *out_executable_filename
);

