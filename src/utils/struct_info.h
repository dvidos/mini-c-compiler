/*
    A construct that supports something like a class info.
    Each item we create via the "new_xxxxxx()" functions could 
    have a pointer to such a structure.

    This structure could then be used to check for equality, compare, hash etc.

    Maybe we can make pseudo-generics by passing the struct_info pointer
    in the container constructor call:
        llist *tokens = new_llist(mempool, token_struct_info);
    Then validations can take place, even debug dump.
    
    Also, if mempools are kept in a central repository list,
    we can create full maps of memory, with all allocated objects,
    if they support this.
*/

#define STRUCT_INFO_MAGIC_NUMBER   0xB6F5536C  // from random.org


typedef struct struct_info {
    int magic_number;    // to verify existence of this structure
    char *struct_name;   // e.g. "token", same with what is given to mpalloc()

    bool (*equals)(void *a, void *b);
    bool (*matches)(void *item, void *match_type, void *match_criteria);
    int (*compare)(void *a, void *b);
    unsigned long (*hash)(void *item);
    char *(*to_string)(mempool *mempool, void *item);
} struct_info;


// usage:
struct_info str_info_data = {
    .magic_number = STRUCT_INFO_MAGIC_NUMBER,
    .struct_name = "str",
    .equals = str_equals,
    .matches = str_matches,
    .compare = str_compare,
    .hash = str_hash,
    .to_string = str_to_string
};
struct_info *str_info = &str_info_data;


str *new_str() {
    void *p = mpalloc();
    p->struct_info = str_info;
    ...
    return p;
}

some_func() {
    bool exists = llist_contains(list, match_type, match_criteria);
    void *target = llist_find_first(list, match_type, match_criteria);
}

// -------------------------------------------------

static struct_info *discover_struct_info(void *instance) {
    if (instance == NULL) return NULL;

    // assume the first item in structure points to the struct_info
    struct_info *info = (struct_info *)(*instance);
    if (info == NULL || info->magic_number != STRUCT_INFO_MAGIC_NUMBER)
        return NULL;
    
    return info;
}

bool is_struct_info_instance(void *instance) {
    return discover_struct_info(instance) != NULL;
}

const char *struct_info_get_struct_name(void *instance) {
    struct_info *info = discover_struct_info(instance);
    if (info == NULL) return NULL;
    return info->struct_name;
}

bool struct_info_is_named_struct(void *instance, char *name) {
    struct_info *info = discover_struct_info(instance);
    if (info == NULL || info->struct_name == NULL) return false;
    return strcmp(info->struct_name, name) == 0;
}

int struct_info_compare_instances(void *a, void *b) {
    struct_info *info = discover_struct_info(a);
    if (info == NULL) return 0;
    return info->compare(a, b);
}

bool struct_info_instances_equal(void *a, void *b) {
    struct_info *info = discover_struct_info(a);
    if (info == NULL) return false;
    if (!struct_info_is_named_struct(b, info->struct_name))
        return false;
    return info->equals(a, b);
}

bool struct_info_instance_matches(void *item, void *match_type, void *match_criteria) {
    struct_info *info = discover_struct_info(a);
    if (info == NULL) return false;
    return info->matches(item, match_type, match_criteria);
}

unsigned long struct_info_hash(void *item) {
    struct_info *info = discover_struct_info(a);
    if (info == NULL) return 0;
    return info->hash(item);
}

char *struct_info_hash(mempool *mempool, void *item) {
    struct_info *info = discover_struct_info(a);
    if (info == NULL) return NULL;
    return info->to_string(mempool, item);
}

