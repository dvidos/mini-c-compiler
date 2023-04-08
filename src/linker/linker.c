#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "../err_handler.h"
#include "../utils.h"
#include "obj_code.h"
#include "elf/elf_contents.h"
#include "elf/elf.h"

// we need to be able to load libraries as well,
// maybe through an offset???

obj_code *load_object_file(char *filename) {
    elf_contents *c = read_elf_file(filename);
    // verify obj file
    // transfer from elf to obj
    return NULL;
}

bool save_object_file(obj_code *obj, char *filename) {
    elf_contents *c = read_elf_file(filename);
    // verify obj file
    // transfer from obj to elf
    long bytes_written;
    return write_elf_file(c, filename, &bytes_written);
}




// arrange code, ro_data, data, bss, etc, align in 4k pages, write 
// relocate as needed, resolve references, write elf file
void x86_link(
    obj_code **objs_array,
    int objs_count,
    u64 base_address,
    char *out_executable_filename
) {
    // in theory many modules, merge them together, update references and symbol addresses
    // map: code, then data, then bss
    // decide base addresses, resolve references.
    // find "_start" as entry point set it
    // save executable

    // single obj for now.
    obj_code *obj = objs_array[0];
    struct symbol_entry *start = obj->symbols->find(obj->symbols, "_start");
    if (start == NULL) {
        error(NULL, 0, "Entry point '_start' not found in obj_code");
        return;
    }
    
    u64 data_base_address;
    u64 bss_base_address;

    data_base_address = round_up(base_address + obj->text_seg->length, 4096);
    bss_base_address = round_up(data_base_address + obj->data_seg->length, 4096);

    if (!obj->relocations->backfill_buffer(
        obj->relocations, 
        obj->symbols,
        obj->text_seg,
        // choose base, depending on the symbol type
        base_address, data_base_address, bss_base_address))
    {
        error(NULL, 0, "Error resolving references");
        return;
    }

    // let's save things
    elf_contents elf;
    elf.flags.is_static_executable = true;
    elf.flags.is_64_bits = false;
    elf.code_address = base_address;
    elf.code_contents = obj->text_seg->buffer;
    elf.code_size = obj->text_seg->length;
    elf.code_entry_point = base_address + start->offset; // address of _start, actually...
    elf.data_address = data_base_address;
    elf.data_contents = obj->data_seg->buffer;
    elf.data_size = obj->data_seg->length;
    elf.bss_address = bss_base_address;
    elf.bss_size = obj->bss_seg->length;
    
    long elf_size = 0;
    if (!write_elf_file(&elf, out_executable_filename, &elf_size)) {
        error(NULL, 0, "Error writing output elf file \"%s\"!\n", out_executable_filename);
        return;
    }

    printf("Wrote %ld bytes to file '%s'\n", elf_size, out_executable_filename);
}

