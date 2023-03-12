#include <stdlib.h>
#include <stddef.h>
#include "../err_handler.h"
#include "module.h"


// arrange code, ro_data, data, bss, etc, align in 4k pages, write 
// relocate as needed, resolve references, write elf file
void x86_link(
    module **modules_arr,
    int modules_length,
    u64 base_address,
    char *out_executable_filename
) {
    error(NULL, 0, "Linker not implemented yet!");
}

