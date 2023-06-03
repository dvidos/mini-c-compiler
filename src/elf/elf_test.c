#include <string.h>
#include <stdbool.h>
#include "elf_contents.h"
#include "elf64_contents.h"
#include "obj_module.h"


void perform_elf_test() {
    // load and save a file, then save again, and load again.
    // both files and both obj_modules should be identical.

    mempool *mp = new_mempool();
    elf64_contents *contents = elf64_load_file(mp, "mcc.o");
    obj_module *module = unpack_elf64_contents(new_str(mp, "mcc.o"), contents, mp);
    print_obj_module(module, stdout);

    elf64_contents *contents2 = pack_elf64_object_file(module, mp);
    elf64_save_file("mcc2.o", contents2); // we should do a `readelf` on this.

    elf64_contents *contents3 = elf64_load_file(mp, "mcc.o");
    obj_module *module3 = unpack_elf64_contents(new_str(mp, "mcc.o"), contents3, mp);
    print_obj_module(module3, stdout);
    // assert(obj_module_equals(module, module3));


    return;
}
