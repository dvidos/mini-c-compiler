#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "../err_handler.h"
#include "../options.h"
#include "../utils/list.h"
#include "../utils.h"
#include "obj_code.h"
#include "elf/elf_contents.h"
#include "elf/elf.h"


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

// --------------------------------------------------------------

static obj_code *create_crt0_code() {
    obj_code *code = new_obj_code_module();
    // paste code here or have assembly being encoded.
    // this code MUST expose the "_start" function symbol
    return code;
}

static bool find_named_symbol(list *obj_codes, char *name, int *obj_index, struct symbol_entry **entry) {
    int codes_len = obj_codes->v->length(obj_codes);
    struct symbol_entry *p;

    for (int i = 0; i < codes_len; i++) {
        obj_code *code = obj_codes->v->get(obj_codes, i);
        p = code->symbols->find(code->symbols, name);
        if (p != NULL) {
            *obj_index = i;
            *entry = p;
            return true;
        }
    }

    return false;
}

static bool include_crt0_if_needed(list *obj_codes) {

    // find entry point. if needed include crt0, to call main()
    int entry_point_obj_index;
    struct symbol_entry *entry_point_symbol;
    if (!find_named_symbol(obj_codes, "_start", &entry_point_obj_index, &entry_point_symbol)) {
        if (!find_named_symbol(obj_codes, "main", &entry_point_obj_index, &entry_point_symbol)) {
            error(NULL, 0, "symbol 'main()' not found, cannot link");
            return false;
        }

        obj_codes->v->insert_at(obj_codes, 0, create_crt0_code());
        if (!find_named_symbol(obj_codes, "_start", &entry_point_obj_index, &entry_point_symbol)) {
            error(NULL, 0, "symbol '_start()' was not found, even in crt0, cannot link");
            return false;
        }
    }

    return true;
}

// info parallel to each obj_code module
typedef struct obj_link_info {
    obj_code *module;

    u64 text_base_address;
    u64 data_base_address;
    u64 bss_base_address;
} obj_link_info;


typedef struct link_map {
    list *obj_codes;       // items are of type: "obj_code"
    list *obj_link_infos;  // items are of type: "struct obj_link_info"
    obj_code *merged;      // the code + symbols we'll write to the final elf file

    u64 base_address;
    u64 entry_point;

    u64 text_base_address;
    u64 data_base_address;
    u64 bss_base_address;

    u64 text_total_size;
    u64 data_total_size;
    u64 bss_total_size;
} link_map;


static link_map *create_link_map(list *obj_codes, u64 base_address) {
    link_map *map = malloc(sizeof(link_map));
    memset(map, 0, sizeof(link_map));

    map->obj_link_infos = new_list();
    map->obj_codes = obj_codes;
    map->merged = new_obj_code_module();
    map->base_address = base_address;

    return map;
}

static bool resolve_addresses_and_merge(link_map *map) {
    obj_link_info *info;
    int i;
    int objs_len = map->obj_codes->v->length(map->obj_codes);

    // populate parallel array of obj_link_info objects, for our convenience
    for (i = 0; i < objs_len; i++) {
        info = malloc(sizeof(obj_link_info));
        memset(info, 0, sizeof(obj_link_info));
        info->module = map->obj_codes->v->get(map->obj_codes, i);
        map->obj_link_infos->v->add(map->obj_link_infos, info);
    }

    // round all sections sizes up to 4K
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        info->module->text_seg->round_up(info->module->text_seg, 4096, 0);
        info->module->data_seg->round_up(info->module->data_seg, 4096, 0);
        info->module->bss_seg->round_up(info->module->bss_seg, 4096, 0);
    }

    // the current running address
    u64 addr = map->base_address;

    // find locations of text segments
    map->text_base_address = addr;
    map->text_total_size = 0;
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        info->text_base_address = addr;
        map->text_total_size += info->module->text_seg->length;
        addr += info->module->text_seg->length;
    }

    // find locations of data segments
    map->data_base_address = addr;
    map->data_total_size = 0;
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        info->data_base_address = addr;
        map->data_total_size += info->module->data_seg->length;
        addr += info->module->data_seg->length;
    }

    // find locations of bss segments
    map->bss_base_address = addr;
    map->bss_total_size = 0;
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        info->bss_base_address = addr;
        map->bss_total_size += info->module->bss_seg->length;
        addr += info->module->bss_seg->length;
    }

    // relocate symbol lists to their new target addresses
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        info->module->symbols->offset(info->module->symbols, SB_CODE, info->text_base_address);
        info->module->symbols->offset(info->module->symbols, SB_DATA, info->data_base_address);
        info->module->symbols->offset(info->module->symbols, SB_BSS, info->bss_base_address);
    }

    // reposition relocations to their new code addresses
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        info->module->relocations->offset(info->module->relocations, info->text_base_address);
    }

    // now that symbols have the target address, we could resolve static (module scope) symbols
    // (but we don't support this for now, so we skip it)


    // merge all sections, all symbols and all relocations
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        map->merged->text_seg->append(map->merged->text_seg, info->module->text_seg);
        map->merged->data_seg->append(map->merged->data_seg, info->module->text_seg);
        map->merged->bss_seg->append(map->merged->bss_seg, info->module->text_seg);

        // we have to be careful with symbols, they must not be declared twice
        // ideally we'd merge only public symbols
        int syms_len = info->module->symbols->length;
        for (int j = 0; j < syms_len; j++) {
            struct symbol_entry *sym = &info->module->symbols->symbols[i];
            if (map->merged->symbols->find(map->merged->symbols, sym->name)) {
                error(NULL, 0, "Linker: symbol '%s' already declared", sym->name);
                return false;
            }
            map->merged->symbols->add(map->merged->symbols, sym->name, sym->address, sym->base);
        }

        // also merge all relocations (ideally only the public ones)
        map->merged->relocations->append(map->merged->relocations, info->module->relocations);
    }

    // now that everything is merged in target addresses, we can backfill relocations
    if (!map->merged->relocations->backfill_buffer(map->merged->relocations, map->merged->symbols, map->merged->text_seg)) {
        error(NULL, 0, "Linker: Error resolving references");
        return false;
    }

    // and now we can find the final "_start" entry point
    struct symbol_entry *start = map->merged->symbols->find(map->merged->symbols, "_start");
    if (start == NULL) {
        error(NULL, 0, "Linker: Cannot find entry point '_start'");
        return false;
    }
    map->entry_point = start->address;

    return true;
}


static void print_link_map(link_map *map) {
    // print where each module is to be loaded
    // print each symbol location, per section (functions, data, bss)
}

// arrange code, ro_data, data, bss, etc, align in 4k pages, write 
// relocate as needed, resolve references, write elf file
void x86_link(list *obj_codes, u64 base_address, char *executable_filename) {
    // many modules, merge them together, update references and symbol addresses
    // map: code, then data, then bss
    // decide & assign base addresses, resolve references.
    // find "_start" for entry point, otherwise, find "main" and add a tiny runtime with exit(0)
    // save executable

    link_map *map = create_link_map(obj_codes, base_address);

    if (!include_crt0_if_needed(map->obj_codes))
        return;

    // derive a link_map, and decide and distribute the locations to be loaded
    if (!resolve_addresses_and_merge(map))
        return;

    if (options.verbose) {
        printf("---- Link Map ----\n");
        print_link_map(map);
    }

    // let's save things
    elf_contents elf;
    memset(&elf, 0, sizeof(elf_contents));
    elf.flags.is_static_executable = true;
    elf.flags.is_64_bits = false;
    elf.code_entry_point = map->entry_point;
    elf.code_address     = map->text_base_address;
    elf.data_address     = map->data_base_address;
    elf.bss_address      = map->bss_base_address;
    elf.code_contents    = map->merged->text_seg->buffer;
    elf.code_size        = map->merged->text_seg->length;
    elf.data_contents    = map->merged->data_seg->buffer;
    elf.data_size        = map->merged->data_seg->length;
    elf.bss_size         = map->merged->bss_seg->length;
    
    long elf_size = 0;
    if (!write_elf_file(&elf, executable_filename, &elf_size)) {
        error(NULL, 0, "Error writing output elf file \"%s\"!\n", executable_filename);
        return;
    }

    printf("Wrote %ld bytes to file '%s'\n", elf_size, executable_filename);
}
