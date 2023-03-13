#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "../err_handler.h"
#include "../utils.h"
#include "obj_code.h"
#include "../elf/elf_contents.h"
#include "../elf/elf.h"


// arrange code, ro_data, data, bss, etc, align in 4k pages, write 
// relocate as needed, resolve references, write elf file
void x86_link(
    obj_code **modules_arr,
    int modules_length,
    u64 base_address,
    char *out_executable_filename
) {
    // in theory many modules, merge them together, update references and symbol addresses
    // map: code, then data, then bss
    // decide base addresses, resolve references.
    // find "_start" as entry point set it
    // save executable

    // single mod for now.
    obj_code *mod = modules_arr[0];
    struct symbol_entry *start = mod->symbols->find(mod->symbols, "_start");
    if (start == NULL) {
        error(NULL, 0, "Entry point '_start' not found in obj_code");
        return;
    }
    
    u64 data_base_address;
    u64 bss_base_address;

    data_base_address = round_up(base_address + mod->text_seg->length, 4096);
    bss_base_address = round_up(data_base_address + mod->data_seg->length, 4096);

    if (!mod->relocations->backfill_buffer(
        mod->relocations, 
        mod->symbols,
        mod->text_seg,
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
    elf.code_contents = mod->text_seg->buffer;
    elf.code_size = mod->text_seg->length;
    elf.code_entry_point = base_address + start->offset; // address of _start, actually...
    elf.data_address = data_base_address;
    elf.data_contents = mod->data_seg->buffer;
    elf.data_size = mod->data_seg->length;
    elf.bss_address = bss_base_address;
    elf.bss_size = mod->bss_seg->length;
    
    long elf_size = 0;
    if (!write_elf_file(&elf, out_executable_filename, &elf_size)) {
        error(NULL, 0, "Error writing output elf file \"%s\"!\n", out_executable_filename);
        return;
    }

    printf("Wrote %ld bytes to file '%s'\n", elf_size, out_executable_filename);
}

