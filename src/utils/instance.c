#include "instance.h"

/*
    A construct that supports something like a class info.
    Each item we create via the "new_xxxxxx()" functions could 
    have a pointer to such a structure.

    This structure could then be used to check for equality, compare, hash,
    in container structurs like list, hashtable etc.

    Maybe we can make pseudo-generics by passing the instance_info pointer
    in the container constructor call:
        list *tokens = new_list(mempool, token_struct_info);
    Then validations can take place, even debug dump.
    
    Also, if mempools are kept in a central repository list,
    we can create full maps of memory, with all allocated objects,
    if they support this.

    // usage:
    instance_info str_instance_info = {
        .magic_number = STRUCT_INFO_MAGIC_NUMBER,
        .name = "str",
        .equals = str_equals,
        .matches = str_matches,
        .compare = str_compare,
        .hash = str_hash,
        .to_string = str_to_string
    };
    instance_info *str_info = &str_instance_info;

    str *new_str() {
        void *p = mpalloc();
        p->instance_info = str_info;
        ...
        return p;
    }

    some_func() {
        bool exists = list_contains(list, match_type, match_criteria);
        void *target = list_find_first(list, match_type, match_criteria);
    }

*/

static inline instance_info *_get_instance_info(const void *ptr) {
    if (ptr == NULL) return NULL;

    // assume the first item in structure points to the instance_info
    instance_info *info = (instance_info *)(*ptr);
    if (info == NULL || info->magic_number != STRUCT_INFO_MAGIC_NUMBER)
        return NULL;
    
    return info;
}

bool has_instance_info(const void *ptr) {
    return (_get_instance_info(ptr) != NULL);
}

const char *get_instance_name(const void *ptr) {
    instance_info *info = _get_instance_info(ptr);
    if (info == NULL) return NULL;
    return info->name;
}

bool is_instance_named(const void *ptr, const char *name) {
    instance_info *info = _get_instance_info(ptr);
    if (info == NULL || info->name == NULL) return false;
    return strcmp(info->name, name) == 0;
}

int compare_instances(const void *a, const void *b) {
    if (a == NULL && b == NULL) return 0;
    if (a == NULL && b != NULL) return 1;
    if (a != NULL && b == NULL) return -1;
    instance_info *info = _get_instance_info(a);
    if (info == NULL) return 0;
    if (info->compare == NULL)
        return (a == b);
    
    return info->compare(a, b);
}

bool instances_equal(const void *a, const void *b) {
    if (a == NULL && b == NULL) return true;
    if (a == NULL && b != NULL) return false;
    if (a != NULL && b == NULL) return false;
    instance_info *info_a = _get_instance_info(a);
    nstance_info *info_b = _get_instance_info(b);
    if (info_a == NULL || info_b == NULL) return false;
    if (!instance_info_is_named(b, info_a->name)) return false;
    if (info_a->equals == NULL) 
        return a == b;
    // 
    return info_a->equals(a, b);
}

bool instance_matches(const void *item, const char *match_type, const void *match_criteria) {
    instance_info *info = _get_instance_info(a);
    if (info == NULL || info->matches == NULL) return false;
    return info->matches(item, match_type, match_criteria);
}

unsigned long instance_hash(const void *item) {
    instance_info *info = _get_instance_info(a);
    if (info == NULL || info->hash == NULL) return 0;
    return info->hash(item);
}

const char *instance_to_string(mempool *mp, const void *item) {
    instance_info *info = _get_instance_info(a);
    if (info == NULL) return NULL;
    if (info->to_string == NULL) {
        char *p = mpalloc(mp, 100);
        snprintf(p, 99, "%s[0x%p]", info->name == NULL ? "(noname)" : info->name, item);
        return p;
    }

    return info->to_string(mp, item);
}

#incdef 