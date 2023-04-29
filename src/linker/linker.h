#pragma once
#include "obj_code.h"
#include "../utils/list.h"


obj_code *load_object_file(char *name);
bool save_object_file(obj_code *obj, char *name);

// arrange code, ro_data, data, bss, etc, align in 4k pages, write 
// relocate as needed, resolve references, write elf file
void x86_link(list *obj_codes, u64 base_address, char *executable_filename);

