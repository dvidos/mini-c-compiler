#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "../err_handler.h"
#include "../run_info.h"
#include "../utils.h"
#include "obj_code.h"
#include "../elf/elf_contents.h"
#include "../elf/elf64_contents.h"
#include "../elf/obj_module.h"
#include "../elf/ar.h"

#define SECTION_ROUNDING_VALUE     1  // e.g. between data of different modules
#define GROUP_ROUNDING_VALUE    4096  // e.g. between .text and .data 

#define R_X86_64_PC32   2
#define R_X86_64_PLT32  4

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
    
    // map for loading / merging similar sections of multiple modules
    llist *grouping_keys;                 // item is str
    hashtable *sections_per_group; // item is llist[obj_section]

    mempool *mempool;
    str *error;
};


// -------------------------------------------------

llist *x86_64_std_libraries(mempool *mp) {
    llist *std_libs = new_llist(mp);
    llist_add(std_libs, new_str(mp, "./libruntime64.a"));
    return std_libs;
}

size_t x86_64_std_load_address() {
    return 0x400000; // 4 MB
}

// -------------------------------------------------

static bool add_participant(link2_info *info, obj_module *module) {
    if (module == NULL)
        return false;
    
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
    // printf("Module from library:\n");
    // m->ops->print(m, stdout);

    return add_participant(info, m);
}

static void discover_library_contents(link2_info *info, str *library_path) {
    link2_lib_info *lib_info = mpalloc(info->mempool, link2_lib_info);
    lib_info->pathname = library_path;
    lib_info->archive = ar_open(info->mempool, library_path);
    if (lib_info->archive == NULL)
        return;
    lib_info->entries = ar_get_entries(lib_info->archive, info->mempool);
    lib_info->symbols = ar_get_symbols(lib_info->archive, info->mempool);
    llist_add(info->lib_infos, lib_info);

    // printf("%s module entries\n", str_charptr(lib_info->pathname));
    // ar_print_entries(lib_info->entries, 50, stdout);
    // printf("%s module symbols\n", str_charptr(lib_info->pathname));
    // ar_print_symbols(lib_info->symbols, 50, stdout);
}

static int compare_module_and_exported_symbol_name(obj_module *module, str *sym_name) {
    obj_symbol *sym = module->ops->find_symbol(module, sym_name, true);
    return (sym != NULL) ? 0 : -1; // 0=equals, i.e. found.
}
static obj_symbol *find_symbol(link2_info *info, obj_module *owner, str *name) {
    // find the symbol somewhere, anywhere actually.
    // this should be more sophisticated, actually...
    mempool *scratch = new_mempool();

    // first look at owner module, include local symbols
    if (owner != NULL) {
        obj_symbol *sym = owner->ops->find_symbol(owner, name, false);
        if (sym != NULL)
            return sym;
    }

    // then look at all other participants, global only
    iterator *participants_it = llist_create_iterator(info->participants, scratch);
    for_iterator(obj_module, part, participants_it) {
        obj_symbol *sym = part->ops->find_symbol(part, name, true);
        if (sym != NULL)
            return sym;
    }

    mempool_release(scratch);
    return NULL;
}

static bool check_symbol_is_defined(link2_info *info, obj_module *owner, str *name) {
    // find the symbol somewhere, anywhere actually.
    // this should be more sophisticated, actually...

    // section names will definitely be there in the executable
    if (str_cmps(name, ".text") == 0 || str_cmps(name, ".data") == 0 || 
        str_cmps(name, ".bss")  == 0 || str_cmps(name, ".rodata") == 0)
        return true;

    return find_symbol(info, owner, name) != NULL;
}

static bool check_mark_unresolved_symbol(link2_info *info, obj_module *owner, str *name) {
    bool found = check_symbol_is_defined(info, owner, name);
    if (!found) {
        if (llist_find_first(info->unresolved_symbols, (comparator_func*)str_cmp, name) == -1)
            llist_add(info->unresolved_symbols, name);
    }
    return found;
}

static bool find_unresolvable_relocations_in_list(link2_info *info, obj_module *owner, llist *obj_relocations) {

    bool all_found = true;
    for_list(obj_relocations, obj_relocation, rel) {
        if (!check_symbol_is_defined(info, owner, rel->symbol_name)) {
            if (llist_find_first(info->unresolved_symbols, (comparator_func*)str_cmp, rel->symbol_name) != -1)
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
        for_list (m->sections, obj_section, s) {
            for_list(s->relocations, obj_relocation, r) {
                all_found &= check_mark_unresolved_symbol(info, m, r->symbol_name);
            }
        }
    }

    return all_found;
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
        int index = llist_find_first(lib_info->symbols, (comparator_func*)compare_archive_symbol_name, name);
        if (index >= 0) {
            link2_lib_module_id *mid = mpalloc(info->mempool, link2_lib_module_id);
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
        if (llist_find_first(info->needed_module_ids, (comparator_func*)compare_module_ids, mid) != -1)
            continue;
        
        // add it to the modules to append
        llist_add(info->needed_module_ids, mid);
    }

    return all_found;
}

bool include_library_modules_as_needed(link2_info *info) {
    int tries = 0;
    while (true) {
        llist_clear(info->unresolved_symbols);
        find_unresolved_symbols(info, true);
        if (llist_length(info->unresolved_symbols) == 0)
            break; // no unresolvable symbols!!
        
        if (run_info->options->verbose) {
            printf("Symbols required: %s\n",
                str_charptr(str_join(info->unresolved_symbols, new_str(info->mempool, ", "), info->mempool)));
        }
        
        llist_clear(info->needed_module_ids);
        if (!find_needed_lib_modules(info)) {
            printf("Cannot continue, symbols not resolved...\n");
            return false; // free(mp)
        }

        for_list(info->needed_module_ids, link2_lib_module_id, mid) {
            if (run_info->options->verbose)
                printf("Including module %s : %s\n", str_charptr(mid->lib_info->pathname), str_charptr(mid->entry->filename));
            
            add_participant_from_library(info, mid);
        }

        if (++tries > 1000) {
            printf("Too many symbol resolution efforts, giving up...\n");
            return false;
        }
    }

    return true;
}

static void prepare_grouping_map(link2_info *info) {
    info->grouping_keys = new_llist(info->mempool);
    info->sections_per_group = new_hashtable(info->mempool, 16);

    for_list(info->participants, obj_module, participant) {
        for_list(participant->sections, obj_section, section) {
            // derive key (verbose for debugging / educational  purposes)
            str *key = new_strf(info->mempool, "%s-%s%s%s%s",
                str_charptr(section->name),
                section->flags.init_to_zero ? "NOBITS" : "PROGBITS",
                section->flags.allocate ? "A" : "-",
                section->flags.writable ? "W" : "-",
                section->flags.executable ? "X" : "-");
            
            // printf("Module %s, section %s, key is %s\n", str_charptr(participant->name), str_charptr(section->name), str_charptr(key));
            
            // add to keys, add to sections-per-key, add to target-per-key
            if (!hashtable_contains(info->sections_per_group, key)) {
                llist_add(info->grouping_keys, key);
                hashtable_set(info->sections_per_group, key, new_llist(info->mempool));
            }
            llist_add(hashtable_get(info->sections_per_group, key), section);
        }
    }
}

void distribute_address_to_group(link2_info *info, str *group_key, size_t *address, size_t section_rounding, size_t group_rounding) {
    llist *group_sections = hashtable_get(info->sections_per_group, group_key);
    for_list(group_sections, obj_section, section) {
        section->ops->change_address(section, (long)(*address));
        (*address) += bin_len(section->contents);
        if (section_rounding > 1)
            (*address) = round_up(*address, section_rounding);
    }

    if (group_rounding > 1)
        (*address) = round_up(*address, group_rounding);

}

bool resolve_relocation(link2_info *info, obj_section *sect, obj_relocation *rel, obj_symbol *sym) {
    /*  From https://intezer.com/blog/malware-analysis/executable-and-linkable-format-101-part-3-relocations/
        Num   Name             Size     Calculation
          2   R_X86_64_PC32    dword    S + A - P
          4   R_X86_64_PLT32   dword    L + A - P
        A: Addend of Elfxx_Rela entries.
        L: Section offset or address of the procedure linkage table (PLT, .got.plt).
        P: The section offset or address of the storage unit being relocated, retrieved via r_offset relocation entry’s field.
        S: Relocation entry’s correspondent symbol value.
    */

    if (rel->type == R_X86_64_PC32) {
        u32 value = sym->value + rel->addendum - (sect->address + rel->offset);
        bin_seek(sect->contents, rel->offset);
        bin_write_dword(sect->contents, value);

    } else if (rel->type == R_X86_64_PLT32) {
        u32 value = sym->value + rel->addendum - (sect->address + rel->offset);
        bin_seek(sect->contents, rel->offset);
        bin_write_dword(sect->contents, value);

    } else {
        printf("Unknown relocation type, symbol: '%s', type=%d, addend=%ld\n",
            str_charptr(sym->name), rel->type, rel->addendum);
        return false;
    }


    return true;
}

bool resolve_relocations(link2_info *info) {
    for_list(info->participants, obj_module, participant) {
        for_list(participant->sections, obj_section, sect) {
            for_list(sect->relocations, obj_relocation, rel) {
                obj_symbol *sym = find_symbol(info, participant, rel->symbol_name);
                if (sym == NULL) {
                    printf("Failed finding symbol '%s', that should no happen...\n", str_charptr(rel->symbol_name));
                    return false;
                }
                if (!resolve_relocation(info, sect, rel, sym))
                    return false;
            }
            // we are done with these relocations, clear list to avoid noise
            llist_clear(sect->relocations);
        }
    }
    return true;
}

static obj_section *merge_grouped_sections(link2_info *info, str *grouping_key, size_t rounding_value) {
    llist *group_sections = hashtable_get(info->sections_per_group, grouping_key);
    obj_section *first = llist_get(group_sections, 0);

    obj_section *target_section = new_obj_section(info->mempool);
    target_section->name = first->name;
    target_section->flags = first->flags;
    target_section->address = first->address;

    for_list(group_sections, obj_section, sect)
        target_section->ops->append(target_section, sect, SECTION_ROUNDING_VALUE);

    size_t len = bin_len(target_section->contents);
    if (rounding_value > 1) {
        len = round_up(len, rounding_value);
        bin_pad(target_section->contents, 0, len);
    }

    return target_section;
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

    // find undefined symbols, look them up in libaries
    if (!include_library_modules_as_needed(info))
        return false;

    // being here it means we have no unresolved symbols!
    // create a hashmap of similar groups of sections (e.g. all .text's from 3 modules)
    prepare_grouping_map(info);
    
    // now that grouping is defined, distribute final addresses
    size_t address = info->base_address;
    for_list(info->grouping_keys, str, key)
        distribute_address_to_group(info, key, &address, SECTION_ROUNDING_VALUE, GROUP_ROUNDING_VALUE);

    // since we have addresses, resolve relocations for all modules / sections
    // keep individual modules intact, to allow private / public symbol resolution.
    if (!resolve_relocations(info))
        return false;

    // now that individual module visibility is not needed,
    // merge all the participating sections together (e.g all .text's and all .data's)
    for_list(info->grouping_keys, str, key)
        llist_add(info->target_module->sections, merge_grouped_sections(info, key, GROUP_ROUNDING_VALUE));

    // debugging purposes
    if (run_info->options->verbose) {
        printf("Merged and rellocated executable module:\n");
        info->target_module->ops->print(info->target_module, true, stdout);
    }

    // finally convert to executable ELF and save
    elf64_contents *elf64_cnt = info->target_module->ops->prepare_elf_contents(info->target_module, ELF_TYPE_EXEC, info->mempool);
    if (elf64_cnt == NULL) {
        printf("Failed generating executable elf contents\n");
        return false;
    }

    if (run_info->options->verbose) {
        printf("Generated ELF contents from module:\n");
        elf64_cnt->ops->print(elf64_cnt, stdout);
    }

    if (!elf64_cnt->ops->save(elf64_cnt, info->executable_path)) {
        printf("Failed saving executable file\n");
        return false;
    }

    if (run_info->options->verbose) {
        printf("ELF contents as savaed in file:\n");
        elf64_cnt->ops->print(elf64_cnt, stdout);
    }

    return true;
}

bool x86_64_link(llist *obj_modules, llist *obj_file_paths, llist *library_file_paths, u64 base_address, str *executable_path) {
    mempool *mp = new_mempool();

    link2_info *info = mpalloc(mp, link2_info);
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

    // llist_add(obj_file_paths, new_str(mp, "./docs/link-sample/file1.o"));
    // llist_add(obj_file_paths, new_str(mp, "./docs/link-sample/file2.o"));
    llist_add(obj_file_paths, new_str(mp, "./src/runtimes/example.o"));

    x86_64_link(modules, obj_file_paths, 
        x86_64_std_libraries(mp), x86_64_std_load_address(), 
        new_str(mp, "b.out"));

    mempool_release(mp);
}
