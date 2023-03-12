#include "module.h"


// arrange code, ro_data, data, bss, etc, align in 4k pages, write 
// relocate as needed, resolve references, write elf file
void x86_linker(
    module **modules_arr,
    int modules_length,
    u64 base_address,
    char *out_filename
);
