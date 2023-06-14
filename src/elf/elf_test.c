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
    elf64_contents_print(contents, stdout);

    obj_module *module = new_obj_module_from_elf64_contents(new_str(mp, "obj.o"), contents, mp);
    obj_module_print(module, stdout);

    // could / should also pack the module and compare with the original elf contents
    elf64_contents *new_contents = pack_elf64_object_file(module, mp);
    // ...

    mempool_release(mp);
}

static void demo_lib_entry(archive *a, llist *entries, int entry_no) {
    archive_entry *e = llist_get(entries, entry_no);
    if (e == NULL)
        return;

    mempool *mp = new_mempool();

    printf("----------- %s : %s (#%d) -----------\n", 
            str_charptr(a->filename), str_charptr(e->filename), entry_no);

    bin *data = ar_read_file(a, e);

    elf64_contents *contents = new_elf64_contents_from_binary(mp, data);
    elf64_contents_print(contents, stdout);

    obj_module *module = new_obj_module_from_elf64_contents(new_str(mp, "obj.o"), contents, mp);
    obj_module_print(module, stdout);

    // could / should also pack the module and compare with the original elf contents
    elf64_contents *new_contents = pack_elf64_object_file(module, mp);
    // ...

    mempool_release(mp);
}


void perform_elf_test() {
    mempool *mp = new_mempool();

    // load and save a file, then save again, and load again.
    // both files and both obj_modules should be identical.

    demo_obj_file("mcc.o");
    demo_obj_file("/usr/lib/x86_64-linux-gnu/crt1.o");
    demo_obj_file("/usr/lib/x86_64-linux-gnu/crti.o");
    demo_obj_file("/usr/lib/x86_64-linux-gnu/crtn.o");
    

    // test the C library
    str *libc_name = new_str(mp, "/usr/lib/x86_64-linux-gnu/libc.a");
    archive *libc = ar_open(mp, libc_name);
    llist *entries = ar_get_entries(libc);
    ar_print_entries(entries, 50, stdout);

    // first file: "/", second file: "//", rest have file names (e.g. "getitimer.o/")
    // second file + are ELF files
    demo_lib_entry(libc, entries, 2);
    demo_lib_entry(libc, entries, 3);
    demo_lib_entry(libc, entries, 4);
    demo_lib_entry(libc, entries, 45);
    demo_lib_entry(libc, entries, 46);
    demo_lib_entry(libc, entries, 47);
    demo_lib_entry(libc, entries, 48);
    demo_lib_entry(libc, entries, 49);


    // what about these? what's the format?
    bin *f0 = ar_read_file(libc, llist_get(entries, 0));
    printf("---- libc entry 0 ----\n");
    bin_print_hex(f0, 2, 0, 256, stdout);

    // seems to be a string table, where the end of each string is 0x0A
    bin *f1 = ar_read_file(libc, llist_get(entries, 1));
    printf("---- libc entry 1 ----\n");
    bin_print_hex(f1, 2, 0, 256, stdout);


    ar_close(libc);
}