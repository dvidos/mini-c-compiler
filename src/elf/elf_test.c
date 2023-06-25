#include <string.h>
#include <stdbool.h>
#include "elf_contents.h"
#include "elf64_contents.h"
#include "obj_module.h"
#include "ar.h"

static void demo_obj_file(char *filename) {
    mempool *mp = new_mempool();

    printf("----------- %s -----------\n", filename);

    str *name = new_str(mp, filename);
    bin *data = new_bin_from_file(mp, name);

    elf64_contents *contents = new_elf64_contents_from_binary(mp, data);
    contents->ops->print(contents, stdout);

    obj_module *module = new_obj_module_from_elf64_contents(contents, mp);
    module->ops->print(module, stdout);
    
    // could / should also pack the module and compare with the original elf contents
    elf64_contents *new_contents = module->ops->pack_object_file(module, mp);
    // ...

    mempool_release(mp);
}

static void demo_lib_entry(archive *a, llist *entries, int entry_no) {
    archive_entry *e = llist_get(entries, entry_no);
    if (e == NULL)
        return;

    mempool *mp = new_mempool();

    printf("----------- Library %s : entry %s (#%d) -----------\n", 
            str_charptr(a->filename), str_charptr(e->filename), entry_no);

    bin *data = ar_load_file_contents(a, e);

    elf64_contents *contents = new_elf64_contents_from_binary(mp, data);
    contents->ops->print(contents, stdout);

    obj_module *module = new_obj_module_from_elf64_contents(contents, mp);
    module->ops->print(module, stdout);

    // could / should also pack the module and compare with the original elf contents
    elf64_contents *new_contents = module->ops->pack_object_file(module, mp);
    // ...

    mempool_release(mp);
}

static void demo_lib_file(char *filename) {
    printf("----- Library %s ------\n", filename);

    mempool *mp = new_mempool();
    archive *lib = ar_open(mp, new_str(mp, filename));

    llist *entries = ar_get_entries(lib, mp);
    printf("Entries (%d)\n", llist_length(entries));
    ar_print_entries(entries, 50, stdout);

    llist *symbols = ar_get_symbols(lib, mp);
    printf("Symbols (%d)\n", llist_length(symbols));
    ar_print_symbols(symbols, 50, stdout);

    demo_lib_entry(lib, entries, 0);
    demo_lib_entry(lib, entries, 1);

    ar_close(lib);
    
    mempool_release(mp);
}

void perform_elf_test() {
    mempool *mp = new_mempool();

    // the object code of our very own compiler
    demo_obj_file("mcc.o");
    
    // test std C library and objects
    demo_lib_file("/usr/lib/x86_64-linux-gnu/libc.a");

    demo_obj_file("/usr/lib/x86_64-linux-gnu/crt1.o");
    // demo_obj_file("/usr/lib/x86_64-linux-gnu/crti.o");
    // demo_obj_file("/usr/lib/x86_64-linux-gnu/crtn.o");

    // our runtime library and objects
    demo_lib_file("./src/runtimes/libruntime.a");
    demo_obj_file("./src/runtimes/rt64.o");
}
