#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "../err_handler.h"
#include "../options.h"
#include "../utils/list.h"
#include "../utils.h"
#include "obj_code.h"
#include "../elf/elf_contents.h"
#include "../elf/elf64_contents.h"


static obj_code *create_crt0_code() {
    obj_code *code = new_obj_code();
    code->vt->set_name(code, "crt0");

    code->text->contents->add_zeros(code->text->contents, 64);
    code->text->symbols->add(code->text->symbols, "_start", 16, 0, ST_FUNCTION, true);
    // we should also have "main" as relocation, in "call main()"

    // paste code here or have assembly being encoded.
    // this code MUST expose the "_start" function symbol
    return code;
}

static bool find_named_symbol(list *obj_codes, char *name, int *obj_index, struct symbol_entry **entry) {
    int codes_len = obj_codes->v->length(obj_codes);
    struct symbol_entry *p;

    for (int i = 0; i < codes_len; i++) {
        obj_code *code = obj_codes->v->get(obj_codes, i);
        p = code->text->symbols->find(code->text->symbols, name);
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
            error(NULL, 0, "symbol '_start' was not found, even in crt0, cannot link");
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
    map->merged = new_obj_code();
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
        info->module->text->contents->round_up(info->module->text->contents, 4096, 0);
        info->module->data->contents->round_up(info->module->data->contents, 4096, 0);
        info->module->bss->contents->round_up(info->module->bss->contents, 4096, 0);
    }

    // the current running address
    u64 addr = map->base_address;

    // find locations of text segments
    map->text_base_address = addr;
    map->text_total_size = 0;
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        info->text_base_address = addr;
        map->text_total_size += info->module->text->contents->length;
        addr += info->module->text->contents->length;
    }

    // find locations of data segments
    map->data_base_address = addr;
    map->data_total_size = 0;
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        info->data_base_address = addr;
        map->data_total_size += info->module->data->contents->length;
        addr += info->module->data->contents->length;
    }

    // find locations of bss segments
    map->bss_base_address = addr;
    map->bss_total_size = 0;
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        info->bss_base_address = addr;
        map->bss_total_size += info->module->bss->contents->length;
        addr += info->module->bss->contents->length;
    }

    // relocate symbol lists to their new target addresses
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        info->module->text->symbols->offset(info->module->text->symbols, info->text_base_address);
        info->module->data->symbols->offset(info->module->data->symbols, info->data_base_address);
        info->module->bss->symbols->offset(info->module->bss->symbols, info->bss_base_address);
    }

    // reposition relocations to their new code addresses
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        info->module->text->relocations->offset(info->module->text->relocations, info->text_base_address);
    }

    // now that symbols have the target address, we could resolve static (module scope) symbols
    // (but we don't support this for now, so we skip it)


    // merge all sections, all symbols and all relocations
    for (i = 0; i < objs_len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        map->merged->text->contents->append(map->merged->text->contents, info->module->text->contents);
        map->merged->data->contents->append(map->merged->data->contents, info->module->text->contents);
        map->merged->bss->contents->append(map->merged->bss->contents, info->module->text->contents);

        // we have to be careful with symbols, they must not be declared twice
        // ideally we'd merge only public symbols
        int syms_len = info->module->text->symbols->length;
        for (int j = 0; j < syms_len; j++) {
            struct symbol_entry *sym = &info->module->text->symbols->symbols[j];
            if (map->merged->text->symbols->find(map->merged->text->symbols, sym->name)) {
                error(NULL, 0, "Linker: symbol '%s' already declared", sym->name);
                return false;
            }
            map->merged->text->symbols->add(map->merged->text->symbols, 
                sym->name, sym->address, sym->size, sym->type, sym->global);
        }

        // also merge all relocations (ideally only the public ones)
        map->merged->text->relocations->append(map->merged->text->relocations, info->module->text->relocations, 0);
    }

    // now that everything is merged in target addresses, we can backfill relocations
    if (!map->merged->text->relocations->backfill_buffer(map->merged->text->relocations, map->merged->text->symbols, map->merged->text->contents)) {
        error(NULL, 0, "Linker: Error resolving references");
        return false;
    }

    // and now we can find the final "_start" entry point
    struct symbol_entry *start = map->merged->text->symbols->find(map->merged->text->symbols, "_start");
    if (start == NULL) {
        error(NULL, 0, "Linker: Cannot find entry point '_start'");
        return false;
    }
    map->entry_point = start->address;

    return true;
}

static void print_link_map(link_map *map, FILE *f) {
    obj_link_info *info;

    // print where each module is to be loaded
    // print each symbol location, per section (functions, data, bss)
    
    // Section         Offset      Length
    // .text       0x00000000  0x00000000 

    fprintf(f, "Section         Offset      Length\n");
    fprintf(f, "%-10s  0x%08lx  0x%08lx\n", ".text", map->text_base_address, map->text_total_size);
    fprintf(f, "%-10s  0x%08lx  0x%08lx\n", ".data", map->data_base_address, map->data_total_size);
    fprintf(f, "%-10s  0x%08lx  0x%08lx\n", ".bss",  map->bss_base_address,  map->bss_total_size);
    fprintf(f, "\n");
    fprintf(f, "Program entry point: 0x%08lx\n", map->entry_point);
    fprintf(f, "\n");

    // a list of modules
    // Module         Text Addr   Size     Data Addr   Size      Bss Addr   Size
    // crt0          0x00000000  1234K    0x00000000  1234K    0x00000000  1234K
    // 123456789012  0x00000000  1234K    0x00000000  1234K    0x00000000  1234K

    fprintf(f, "Module         Text Addr   Size     Data Addr   Size      Bss Addr   Size\n");
    int len = map->obj_link_infos->v->length(map->obj_link_infos);
    for (int i = 0; i < len; i++) {
        // section num, address and size for each section
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        fprintf(f, "%-12s  0x%08lx  %4dK  0x%08lx  %4dK  0x%08lx  %4dK\n",
            info->module->name == NULL ? "" : info->module->name,
            info->text_base_address,
            info->module->text->contents->length / 1024,
            info->data_base_address,
            info->module->data->contents->length / 1024,
            info->bss_base_address,
            info->module->bss->contents->length / 1024
        );
    }
    fprintf(f, "\n");

    // a list of symbols (maybe also the module each symbol comes from???)
    // Section  Module           Address  Symbol                Type
    // 1234567  123456789012  0x00000000  12345678901234567890  XXXX
    fprintf(f, "Section  Module           Address  Symbol                Type\n");

    // going over the modules to allow printing of the module name
    len = map->obj_link_infos->v->length(map->obj_link_infos);
    bool section_shown = false;
    obj_code *last_module = NULL;
    for (int i = 0; i < len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        int j_len = info->module->text->symbols->length;
        for (int j = 0; j < j_len; j++) {
            struct symbol_entry *sym = &info->module->text->symbols->symbols[j];
            fprintf(f, "%-7s  %-12s  0x%08lx  %-20s  %-4s\n",
                section_shown ? "" : ".text",
                info->module == last_module ? "" : info->module->name,
                sym->address,
                sym->name,
                ""
            );
        }
        section_shown = true;
        last_module = info->module;
    }
    section_shown = false;
    last_module = NULL;
    for (int i = 0; i < len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        int j_len = info->module->data->symbols->length;
        for (int j = 0; j < j_len; j++) {
            struct symbol_entry *sym = &info->module->data->symbols->symbols[j];
            fprintf(f, "%-7s  %-12s  0x%08lx  %-20s  %-4s\n",
                section_shown ? "" : ".data",
                info->module == last_module ? "" : info->module->name,
                sym->address,
                sym->name,
                ""
            );
        }
        section_shown = true;
        last_module = info->module;
    }
    section_shown = false;
    last_module = NULL;
    for (int i = 0; i < len; i++) {
        info = map->obj_link_infos->v->get(map->obj_link_infos, i);
        int j_len = info->module->bss->symbols->length;
        for (int j = 0; j < j_len; j++) {
            struct symbol_entry *sym = &info->module->bss->symbols->symbols[j];
            fprintf(f, "%-7s  %-12s  0x%08lx  %-20s  %-4s\n",
                section_shown ? "" : ".bss",
                info->module == last_module ? "" : info->module->name,
                sym->address,
                sym->name,
                ""
            );
        }
        section_shown = true;
        last_module = info->module;
    }
    fprintf(f, "\n");
    fprintf(f, "\n");
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
        print_link_map(map, stdout);
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
    elf.code_contents    = map->merged->text->contents->buffer;
    elf.code_size        = map->merged->text->contents->length;
    elf.data_contents    = map->merged->data->contents->buffer;
    elf.data_size        = map->merged->data->contents->length;
    elf.bss_size         = map->merged->bss->contents->length;
    



    mempool *mp = new_mempool();
    elf64_contents *contents = new_elf64_contents(mp);
    bool saved = elf64_save_file(executable_filename, contents);
    mempool_release(mp);
    
    if (!saved) {
        error(NULL, 0, "Error writing output elf file \"%s\"!\n", executable_filename);
        return;
    }
    printf("Wrote file '%s'\n", executable_filename);
}
