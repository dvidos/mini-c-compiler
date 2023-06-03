#include <string.h>
#include <stdbool.h>
#include "elf_contents.h"
#include "elf_tools.h"
#include "obj_module.h"


void perform_elf_test() {
    // elf_contents c;
    // memset(&c, 0, sizeof(c));
    // read_elf_file("mcc.o", &c);

    mempool *mp = new_mempool();
    elf64_contents *contents = elf64_load_file(mp, "mcc.o");
    obj_module *mod = unpack_elf64_contents(new_str(mp, "mcc.o"), contents, mp);
    print_obj_module(mod, stdout);

    // then we should try to merge two or more obj_modules


    return;
}
