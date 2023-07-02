#pragma once
#include "obj_code.h"
#include "../utils/data_structs.h"


bool x86_64_link(llist *obj_modules, llist *obj_file_paths, llist *library_file_paths, u64 base_address, str *executable_path);
void link_test();