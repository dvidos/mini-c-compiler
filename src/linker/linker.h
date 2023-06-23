#pragma once
#include "obj_code.h"
#include "../utils/list.h"
#include "../utils/data_structs.h"


bool x86_link_v2(llist *obj_modules, llist *obj_file_paths, llist *library_file_paths, u64 base_address, str *executable_path);
void link_test();