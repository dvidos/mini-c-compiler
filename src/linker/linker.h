#pragma once
#include "obj_code.h"
#include "../utils/all.h"


llist *x86_64_std_libraries(mempool *mp);
size_t x86_64_std_load_address();
bool x86_64_link(llist *obj_modules, llist *obj_file_paths, llist *library_file_paths, u64 base_address, str *executable_path);
void link_test();