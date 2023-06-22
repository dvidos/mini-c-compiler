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
#include "../elf/obj_module.h"
#include "../elf/ar.h"


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
    bool saved = elf64_contents_save(executable_filename, contents);
    mempool_release(mp);
    
    if (!saved) {
        error(NULL, 0, "Error writing output elf file \"%s\"!\n", executable_filename);
        return;
    }
    printf("Wrote file '%s'\n", executable_filename);
}


// ------------------------------------------------------------------------------------



// we cannot have everything in loose variables, prepare something
struct linker2_lib_module_id { // identifies a module in a library
    struct linker2_lib_info *lib_info;
    archive_entry *entry;
};
struct linker2_lib_info {
    str *pathname;
    archive *archive;
    llist *symbols; // items are of type archive_symbol
    llist *entries; // items are of type archive_entry
};
struct linker2_obj_info {
    str *pathname;
    obj_module *loaded_module;
};
struct linker2_info {
    // passed into linker call
    str *executable_path;
    u64 base_address;
    llist *obj_modules;
    llist *obj_file_paths;
    llist *library_file_paths;

    // generated data
    obj_module *target_module;
    llist *obj_infos;     // item type is linker2_obj_info
    llist    *lib_infos;          // item type is linker2_lib_info

    // unresolved symbols and needed modules
    llist    *unresolved_symbols;          // item type is str
    llist    *needed_modules;              // item type is linker2_lib_module_id
    
    mempool *mempool;
    str *error;
};


// ------------------

static bool merge_module(struct linker2_info *info, obj_module *module) {
    // should be smarter about it.
    info->target_module->ops->append(info->target_module, module);
    return true;
}

static bool merge_module_from_file(struct linker2_info *info, str *obj_path) {
    bin *obj_data = new_bin_from_file(info->mempool, obj_path);
    if (obj_data == NULL) // failed reading file
        return false;
    
    elf64_contents *elf_cnt = new_elf64_contents_from_binary(info->mempool, obj_data);
    if (elf_cnt == NULL) // not an ELF file
        return false; 
    
    obj_module *m = new_obj_module_from_elf64_contents(elf_cnt, info->mempool);
    return merge_module(info, m);
}

static bool merge_module_from_library(struct linker2_info *info, struct linker2_lib_module_id *mod_id) {
    bin *mod_data = ar_load_file_contents(mod_id->lib_info->archive, mod_id->entry);
    if (mod_data == NULL) // failed reading file
        return false;
    
    elf64_contents *elf_cnt = new_elf64_contents_from_binary(info->mempool, mod_data);
    if (elf_cnt == NULL) // not an ELF file
        return false; 
    
    obj_module *m = new_obj_module_from_elf64_contents(elf_cnt, info->mempool);
    return merge_module(info, m);
}

static bool discover_library(struct linker2_info *info, str *library_path) {
    struct linker2_lib_info *lib_info = mempool_alloc(info->mempool, sizeof(struct linker2_lib_info), "linker2_lib_info");
    lib_info->pathname = library_path;
    lib_info->archive = ar_open(info->mempool, library_path);
    if (lib_info->archive == NULL)
        return false;
    lib_info->entries = ar_get_entries(lib_info->archive, info->mempool);
    lib_info->symbols = ar_get_symbols(lib_info->archive, info->mempool);
    llist_add(info->lib_infos, lib_info);

    printf("%s module entries\n", str_charptr(lib_info->pathname));
    ar_print_entries(lib_info->entries, 50, stdout);
    printf("%s module symbols\n", str_charptr(lib_info->pathname));
    ar_print_symbols(lib_info->symbols, 50, stdout);

    return true;
}


static bool check_symbol_merged(struct linker2_info *info, str *name) {
    // find the symbol somewhere, anywhere actually.
    // this should be more sophisticated, actually...
    bool found = false;

    for_list(info->target_module->text->symbols, obj_symbol, s) {
        if (str_equals(s->name, name))
            return true;
    }

    return false;
}

static bool find_list_unresolved_symbols(struct linker2_info *info, llist *obj_relocations) {
    bool all_found = true;
    for_list(obj_relocations, obj_relocation, rel) {
        if (!check_symbol_merged(info, rel->symbol_name)) {
            llist_add(info->unresolved_symbols, rel->symbol_name);
            all_found = true;
        }
    }
    return all_found;
}

static bool find_unresolved_symbols(struct linker2_info *info) {
    bool verified = true;
    verified &= find_list_unresolved_symbols(info, info->target_module->text->relocations);
    verified &= find_list_unresolved_symbols(info, info->target_module->data->relocations);
    verified &= find_list_unresolved_symbols(info, info->target_module->bss->relocations);
    verified &= find_list_unresolved_symbols(info, info->target_module->rodata->relocations);
    return verified;
}

static bool backfill_relocations(struct linker2_info *info) {
    // we need to resolve all symbols, consider all libraries
    // for each module we add, we need to go around again, 
    // as the new module may have new unresolved symbols
    // possible explanation of x86_64 relocations:
    // https://sourceware.org/git/?p=binutils-gdb.git;a=blob;f=include/elf/x86-64.h;h=60b3c2ad10e66bb14338bd410c3a7566b09c4eb4;hb=e0ce6dde97881435d33652572789b94c846cacde
    // format of the index is here: https://sourceware.org/git/?p=binutils-gdb.git;a=blob;f=binutils/nm.c;h=f96cfa31cb90ec646ac61a43509806be21e014e2;hb=e0ce6dde97881435d33652572789b94c846cacde#l730
    // 
    // put unresolved symbols in info->unresolved symbols

    llist_clear(info->unresolved_symbols);
    
    // visit each section, visit each relocation note,
    // look for a symbol in all the other sections.
    // but we should not trust addresses, until we have resolved all symbols
    // otherwise, adding a lib module would through addresses off (e.g. rodata)
    
}

static int compare_archive_symbol_name(const archive_symbol *s1, const archive_symbol *s2) {
    return str_cmp(s1->name, s2->name);
}

static bool find_needed_lib_modules(struct linker2_info *info) {
    archive_symbol target;
    bool all_found = true;

    // find modules, put them in info->needed_modules
    llist_clear(info->needed_modules);
    for_list(info->unresolved_symbols, str, unresolved_name) {
        // find the symbol in one of the libraries
        // if found, create a linker2_lib_module_id and add it to the needed_modules
        bool found = false;
        target.name = unresolved_name;
        for_list(info->lib_infos, struct linker2_lib_info, li) {
            // find the symbol
            int index = llist_find_first(li->symbols, (comparator_function*)compare_archive_symbol_name, &target);
            if (index == -1)
                continue;
            
            // it is found
            struct linker2_lib_module_id *id = mempool_alloc(info->mempool, sizeof(struct linker2_lib_module_id), "linker2_lib_module_id");
            id->lib_info = li;
            id->entry = ((archive_symbol *)llist_get(li->symbols, index))->entry;
            found = true;
            break;
        }

        if (!found) {
            printf("Unresolved symbol '%s'\n", str_charptr(unresolved_name));
            all_found = false;
        }
    }

    return all_found;
}

static bool merge_needed_library_modules(struct linker2_info *info) {
    // for each needed modules, load it and merge it into target.
    for_list(info->needed_modules, struct linker2_lib_module_id, id) {
        merge_module_from_library(info, id);
    }
}

bool x86_link_v2(llist *obj_modules, llist *obj_file_paths, llist *library_file_paths, u64 base_address, str *executable_path) {
    mempool *mp = new_mempool();
    bool success = false;

    struct linker2_info *info = mempool_alloc(mp, sizeof(struct linker2_info), "linker2_info");
    info->executable_path = executable_path;
    info->base_address = base_address;
    info->obj_modules = obj_modules;
    info->obj_file_paths = obj_file_paths;
    info->library_file_paths = library_file_paths;
    info->mempool = mp;

    info->target_module = new_obj_module(mp);
    info->obj_infos = new_llist(mp);
    info->lib_infos = new_llist(mp);
    info->unresolved_symbols = new_llist(mp);
    info->needed_modules = new_llist(mp);
    
    // add all raw modules
    for_list(obj_modules, obj_module, m)
        merge_module(info, m);

    // add all specified modules from files
    for_list(obj_file_paths, str, path)
        merge_module_from_file(info, path);

    // load all symbols from all libraries
    for_list(library_file_paths, str, lib_path)
        discover_library(info, lib_path);

    // // now we can resolve symbols
    // while (true) {
    //     if (find_unresolved_symbols(info)) {
    //         backfill_relocations(info);
    //         break;
    //     }

    //     if (!find_needed_lib_modules(info))
    //         return false;
    //     if (llist_length(info->needed_modules) == 0)
    //         return false;

    //     // now bring all the modules and merge them
    //     for_list(info->needed_modules, struct linker2_lib_module_id, mod_id)
    //         merge_module_from_library(info, mod_id);
    // }

    if (success) {
        printf("Executable module:\n");
        info->target_module->ops->print(info->target_module, stdout);

        // finally save the executable... (fingers crossed to be workig)
        elf64_contents *elf64_cnt = info->target_module->ops->pack_executable_file(info->target_module, mp);
        success = elf64_contents_save((char *)str_charptr(executable_path), elf64_cnt);
    }

    mempool_release(mp);
    return success;
}

void link_test() {
    mempool *mp = new_mempool();

    llist *modules = new_llist(mp);
    llist *obj_file_paths = new_llist(mp);
    llist *lib_file_paths = new_llist(mp);

    // llist_add(obj_file_paths, new_str(mp, "./docs/link-sample/file1.o"));
    // llist_add(obj_file_paths, new_str(mp, "./docs/link-sample/file2.o"));
    llist_add(obj_file_paths, new_str(mp, "./src/runtimes/example.o"));
    llist_add(lib_file_paths, new_str(mp, "./src/runtimes/libruntime.a"));

    x86_link_v2(modules, obj_file_paths, lib_file_paths, 0x800000, new_str(mp, "b.out"));

    mempool_release(mp);
}
