#pragma once
#include "obj_code.h"
#include "../utils/list.h"
#include "../utils/data_structs.h"


obj_code *load_object_file(char *name);
bool save_object_file(obj_code *obj, char *name);

// arrange code, ro_data, data, bss, etc, align in 4k pages, write 
// relocate as needed, resolve references, write elf file
void x86_link(list *obj_codes, u64 base_address, char *executable_filename);

bool x86_link_v2(llist *obj_modules, llist *obj_file_paths, llist *library_file_paths, u64 base_address, str *executable_path);
void link_test();