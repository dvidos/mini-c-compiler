#include <string.h>
#include <stdbool.h>
#include "elf_contents.h"
#include "elf64_contents.h"
#include "obj_module.h"
#include "ar.h"


void perform_elf_test() {
    mempool *mp = new_mempool();

    // load and save a file, then save again, and load again.
    // both files and both obj_modules should be identical.

    str *mcc_o_filename = new_str(mp, "mcc.o");
    bin *mcc_o = new_bin_from_file(mp, mcc_o_filename);
    elf64_contents *contents = new_elf64_contents_from_binary(mp, mcc_o);
    elf64_contents_print(contents, stdout);
    obj_module *module = new_obj_module_from_elf64_contents(mcc_o_filename, contents, mp);
    obj_module_print(module, stdout);
    elf64_contents *contents2 = pack_elf64_object_file(module, mp);
    elf64_contents_save("mcc2.o", contents2); // we should do a `readelf` on this.


    // load the crt1 file, that defines the "_start" symbol!
    str *crt1_name = new_str(mp, "/usr/lib/x86_64-linux-gnu/crt1.o");
    bin *crt1_data = new_bin_from_file(mp, crt1_name);
    elf64_contents *crt1_contents = new_elf64_contents_from_binary(mp, crt1_data);
    elf64_contents_print(crt1_contents, stdout);
    obj_module *crt1_module = new_obj_module_from_elf64_contents(new_str(mp, "crt1.o"), crt1_contents, mp);
    obj_module_print(crt1_module, stdout);
    

    // test the C library
    str *libc_name = new_str(mp, "/usr/lib/x86_64-linux-gnu/libc.a");
    archive *libc = ar_open(mp, libc_name);
    llist *entries = ar_get_entries(libc);

    iterator *it = llist_create_iterator(entries, mp);
    printf("   Idx      Offset        Size  File name\n");
    //     "  1234  1234567890  1234567890  123..."
    int idx = 0;
    for_iterator(archive_entry, e, it) {
        printf("  %4d %10ld  %10ld  %s\n", idx++, e->offset, e->size, str_charptr(e->filename));
        if (idx > 30)
            break;
    }

    // first file: "/", second file: "//", rest have file names (e.g. "getitimer.o/")
    // second file + are ELF files
    bin *f0 = ar_read_file(libc, llist_get(entries, 0));
    bin *f1 = ar_read_file(libc, llist_get(entries, 1));
    bin *f2 = ar_read_file(libc, llist_get(entries, 2));
    bin *f3 = ar_read_file(libc, llist_get(entries, 3));
    bin *b45 = ar_read_file(libc, llist_get(entries, 45)); // lc-telephone.o
    bin *b46 = ar_read_file(libc, llist_get(entries, 46)); // "0"
    bin *b47 = ar_read_file(libc, llist_get(entries, 47)); // "18"
    bin *b48 = ar_read_file(libc, llist_get(entries, 48)); // lc-collate.o
    ar_close(libc);


    elf64_contents *lc_collate_contents = new_elf64_contents_from_binary(mp, b48);
    elf64_contents_print(lc_collate_contents, stdout);
    obj_module *lc_collate_module = new_obj_module_from_elf64_contents(new_str(mp, "lc_collate.o"), lc_collate_contents, mp);
    obj_module_print(lc_collate_module, stdout);
    




    return;
}
