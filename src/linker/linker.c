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



// we cannot have everything in loose variables, prepare a hierarchy
typedef struct link2_lib_module_id link2_lib_module_id;
typedef struct link2_lib_info link2_lib_info;
typedef struct link2_obj_info link2_obj_info;
typedef struct link2_info link2_info;

struct link2_lib_module_id { // identifies a module in a library
    link2_lib_info *lib_info;
    archive_entry *entry;
};

struct link2_lib_info {
    str *pathname;
    archive *archive;
    llist *symbols; // items are of type archive_symbol
    llist *entries; // items are of type archive_entry
};

struct link2_obj_info {
    str *pathname;
    obj_module *loaded_module;
};

struct link2_info {
    // passed into linker call
    str *executable_path;
    str *entry_point_name;
    u64 base_address;
    llist *obj_modules;
    llist *obj_file_paths;
    llist *library_file_paths;

    // generated data
    llist *lib_infos;     // item type is link2_lib_info
    llist *participants;  // item type is obj_module
    obj_module *target_module;

    // unresolved symbols and needed modules
    llist *unresolved_symbols;  // item type is str
    llist *needed_module_ids;   // item type is link2_lib_module_id
    
    mempool *mempool;
    str *error;
};


// -------------------------------------------------



static bool add_participant(link2_info *info, obj_module *module) {
    // should be smarter about it.
    // info->target_module->ops->append(info->target_module, module);

    llist_add(info->participants, module);

    return true;
}

static bool add_participant_from_file(link2_info *info, str *obj_path) {
    bin *obj_data = new_bin_from_file(info->mempool, obj_path);
    if (obj_data == NULL) // failed reading file
        return false;
    
    elf64_contents *elf_cnt = new_elf64_contents_from_binary(info->mempool, obj_data);
    if (elf_cnt == NULL) // not an ELF file
        return false; 
    
    obj_module *m = new_obj_module_from_elf64_contents(elf_cnt, info->mempool);
    return add_participant(info, m);
}

static bool add_participant_from_library(link2_info *info, link2_lib_module_id *mod_id) {
    bin *mod_data = ar_load_file_contents(mod_id->lib_info->archive, mod_id->entry);
    if (mod_data == NULL) // failed reading file
        return false;
    
    elf64_contents *elf_cnt = new_elf64_contents_from_binary(info->mempool, mod_data);
    if (elf_cnt == NULL) // not an ELF file
        return false; 
    
    obj_module *m = new_obj_module_from_elf64_contents(elf_cnt, info->mempool);
    return add_participant(info, m);
}

static void discover_library_contents(link2_info *info, str *library_path) {
    link2_lib_info *lib_info = mempool_alloc(info->mempool, sizeof(link2_lib_info), "linker2_lib_info");
    lib_info->pathname = library_path;
    lib_info->archive = ar_open(info->mempool, library_path);
    if (lib_info->archive == NULL)
        return;
    lib_info->entries = ar_get_entries(lib_info->archive, info->mempool);
    lib_info->symbols = ar_get_symbols(lib_info->archive, info->mempool);
    llist_add(info->lib_infos, lib_info);

    /*
        printf("%s module entries\n", str_charptr(lib_info->pathname));
        ar_print_entries(lib_info->entries, 50, stdout);
        printf("%s module symbols\n", str_charptr(lib_info->pathname));
        ar_print_symbols(lib_info->symbols, 50, stdout);
    */
}

static int compare_module_and_exported_symbol_name(obj_module *module, str *sym_name) {
    obj_symbol *sym = module->ops->find_symbol(module, sym_name, true);
    return (sym != NULL) ? 0 : -1; // 0=equals, i.e. found.
}

static bool check_symbol_is_defined(link2_info *info, obj_module *owner, str *name) {
    // find the symbol somewhere, anywhere actually.
    // this should be more sophisticated, actually...

    // section names will definitely be there in the executable
    if (str_cmps(name, ".text") == 0 || str_cmps(name, ".data") == 0 || 
        str_cmps(name, ".bss")  == 0 || str_cmps(name, ".rodata") == 0)
        return true;

    // first look at owner module, include local symbols
    if (owner != NULL) {
        obj_symbol *sym = owner->ops->find_symbol(owner, name, false);
        if (sym != NULL)
            return true;
    }

    // then look at all other participants, global only
    int index = llist_find_first(info->participants, 
        (comparator_function*)compare_module_and_exported_symbol_name, name);
    if (index != -1)
        return true;

    // not found anywhere
    return false;
}


static bool check_mark_unresolved_symbol(link2_info *info, obj_module *owner, str *name) {
    bool found = check_symbol_is_defined(info, owner, name);
    if (!found) {
        if (llist_find_first(info->unresolved_symbols, (comparator_function*)str_cmp, name) == -1)
            llist_add(info->unresolved_symbols, name);
    }
    return found;
}

static bool find_unresolvable_relocations_in_list(link2_info *info, obj_module *owner, llist *obj_relocations) {

    bool all_found = true;
    for_list(obj_relocations, obj_relocation, rel) {
        if (!check_symbol_is_defined(info, owner, rel->symbol_name)) {
            if (llist_find_first(info->unresolved_symbols, (comparator_function*)str_cmp, rel->symbol_name) != -1)
                continue;
            llist_add(info->unresolved_symbols, rel->symbol_name);
            all_found = false;
        }
    }
    return all_found;
}

static bool find_unresolved_symbols(link2_info *info, bool check_startup_symbol) {
    bool all_found = true;

    if (check_startup_symbol) {
        all_found &= check_mark_unresolved_symbol(info, NULL, info->entry_point_name);
    }

    for_list (info->participants, obj_module, m) {
        for_list(m->text->relocations, obj_relocation, r)
            all_found &= check_mark_unresolved_symbol(info, m, r->symbol_name);

        for_list(m->data->relocations, obj_relocation, r)
            all_found &= check_mark_unresolved_symbol(info, m, r->symbol_name);

        for_list(m->bss->relocations, obj_relocation, r)
            all_found &= check_mark_unresolved_symbol(info, m, r->symbol_name);

        for_list(m->rodata->relocations, obj_relocation, r)
            all_found &= check_mark_unresolved_symbol(info, m, r->symbol_name);
    }

    return all_found;
}

static bool backfill_relocations(link2_info *info) {
    // possible explanation of x86_64 relocations:
    // https://sourceware.org/git/?p=binutils-gdb.git;a=blob;f=include/elf/x86-64.h;h=60b3c2ad10e66bb14338bd410c3a7566b09c4eb4;hb=e0ce6dde97881435d33652572789b94c846cacde

    // if using the "tagret" module, should go over all sections, 
    // find the symbols, and update relocation.
    // but I think we should have a collection of modules,
    // to be able to resolve local and lobal symbols.
}

static int compare_archive_symbol_name(const archive_symbol *s1, str *name) {
    return str_cmp(s1->name, name);
}

static int compare_module_ids(const link2_lib_module_id *m1, const link2_lib_module_id *m2) {
    int c;

    // easy case compare references
    if (m1 == m2) return 0;
    if (m1->lib_info == m2->lib_info && m1->entry == m2->entry) return 0;
    
    // see if same lib info by values
    c = str_cmp(m1->lib_info->pathname, m2->lib_info->pathname);
    if (c != c) return c;

    // so same libraries, see if same module
    if (m1->entry == m2->entry) return 0;
    return str_cmp(m1->entry->filename, m2->entry->filename);
}

static link2_lib_module_id *find_lib_module_for_symbol(link2_info *info, str *name) {
    // check all considered libraries
    for_list(info->lib_infos, link2_lib_info, lib_info) {
        // see if this library contains this symbol
        int index = llist_find_first(lib_info->symbols, (comparator_function*)compare_archive_symbol_name, name);
        if (index >= 0) {
            link2_lib_module_id *mid = mempool_alloc(info->mempool, sizeof(link2_lib_module_id), "linker2_lib_module_id");
            mid->lib_info = lib_info;
            mid->entry = ((archive_symbol *)llist_get(lib_info->symbols, index))->entry;
            return mid;
        }
    }

    return NULL;
}

static bool find_needed_lib_modules(link2_info *info) {
    bool all_found = true;

    for_list(info->unresolved_symbols, str, unresolved_name) {
        link2_lib_module_id *mid = find_lib_module_for_symbol(info, unresolved_name);
        if (mid == NULL) {
            printf("Cannot find symbol '%s' in any library\n", str_charptr(unresolved_name));
            all_found = false;
            continue;
        }
        
        // maybe module already considered, 
        if (llist_find_first(info->needed_module_ids, (comparator_function*)compare_module_ids, mid) != -1)
            continue;
        
        // add it to the modules to append
        llist_add(info->needed_module_ids, mid);
    }

    return all_found;
}

static bool do_link2(link2_info *info) {
    // add all raw modules
    for_list(info->obj_modules, obj_module, m)
        add_participant(info, m);

    // add all specified modules from files
    for_list(info->obj_file_paths, str, path)
        add_participant_from_file(info, path);

    // load all symbols from all libraries
    for_list(info->library_file_paths, str, lib_path)
        discover_library_contents(info, lib_path);

    int tries = 0;
    while (true) {
        llist_clear(info->unresolved_symbols);
        find_unresolved_symbols(info, true);
        if (llist_length(info->unresolved_symbols) == 0)
            break;
        
        printf("we need to find the following symbols in libraries\n");
        for_list(info->unresolved_symbols, str, s)
            printf("- %s\n", str_charptr(s));

        llist_clear(info->needed_module_ids);
        if (!find_needed_lib_modules(info)) {
            printf("Cannot continue, unlinkable symbols...\n");
            return false; // free(mp)
        }

        for_list(info->needed_module_ids, link2_lib_module_id, mid) {
            printf("merging module '%s' from library '%s'\n", str_charptr(mid->entry->filename), str_charptr(mid->lib_info->pathname));
            add_participant_from_library(info, mid);
        }

        if (++tries > 1000) {
            printf("Too many symbol resolution efforts, aborting...\n");
            return false;
        }
    }

    printf("If we got here, all symbols are present and relocatable!!!!!\n");
    backfill_relocations(info);

    printf("Executable module:\n");
    info->target_module->ops->print(info->target_module, stdout);

    // finally save the executable... (fingers crossed to be working)
    elf64_contents *elf64_cnt = info->target_module->ops->pack_executable_file(info->target_module, info->mempool);
    if (elf64_cnt == NULL) {
        printf("Failed generating executable elf\n");
        return false;
    }
    if (!elf64_contents_save((char *)str_charptr(info->executable_path), elf64_cnt)) {
        printf("Failed saving executable file\n");
        return false;
    }

    return true;
}

bool x86_link_v2(llist *obj_modules, llist *obj_file_paths, llist *library_file_paths, u64 base_address, str *executable_path) {
    mempool *mp = new_mempool();

    link2_info *info = mempool_alloc(mp, sizeof(link2_info), "link2_info");
    info->executable_path = executable_path;
    info->entry_point_name = new_str(mp, "_start");
    info->base_address = base_address;
    info->obj_modules = obj_modules;
    info->obj_file_paths = obj_file_paths;
    info->library_file_paths = library_file_paths;
    info->lib_infos = new_llist(mp);
    info->participants = new_llist(mp);
    info->target_module = new_obj_module(mp);
    info->unresolved_symbols = new_llist(mp);
    info->needed_module_ids = new_llist(mp);
    info->mempool = mp;

    bool success = do_link2(info);

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
