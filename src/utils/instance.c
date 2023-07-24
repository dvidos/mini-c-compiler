#include <string.h>
#include "instance.h"
#include "../utils.h"

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
    instance_info *info = (instance_info *)(*(instance_info **)ptr);
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
    instance_info *info_b = _get_instance_info(b);
    if (info_a == NULL || info_b == NULL) return false;
    if (!is_instance_named(b, info_a->name)) return false;
    if (info_a->equals == NULL) 
        return a == b;
    
    return info_a->equals(a, b);
}

bool instance_matches(const void *item, const char *match_type, const void *match_criteria) {
    instance_info *info = _get_instance_info(item);
    if (info == NULL || info->matches == NULL) return false;
    return info->matches(item, match_type, match_criteria);
}

unsigned long instance_hash(const void *item) {
    instance_info *info = _get_instance_info(item);
    if (info == NULL || info->hash == NULL) return 0;
    return info->hash(item);
}

const char *instance_to_string(mempool *mp, const void *item) {
    instance_info *info = _get_instance_info(item);
    if (info == NULL) return NULL;
    if (info->to_string == NULL) {
        char *p = mpalloc(mp, 100);
        snprintf(p, 99, "%s[0x%p]", info->name == NULL ? "(noname)" : info->name, item);
        return p;
    }

    return info->to_string(mp, item);
}


#ifdef INCLUDE_UNIT_TESTS
static bool cmd_equals(const void *a, const void *b);
static bool cmd_matches(const void *item, const char *match_type, const void *match_criteria);
static int cmd_compare(const void *a, const void *b);
static unsigned long cmd_hash(const void *item);
static char *cmd_to_string(mempool *mp, const void *item);

struct instance_info cmd_info = {
    .magic_number = STRUCT_INFO_MAGIC_NUMBER,
    .name = "cmd",
    .equals = cmd_equals,
    .matches = cmd_matches,
    .compare = cmd_compare,
    .hash = cmd_hash,
    .to_string = cmd_to_string,
};
struct cmd { // sample struct
    instance_info *instance_info;
    char *name;
    int arg1;
    int arg2;
};
struct simple_cmd {
    char *name;
    int arg1;
    int arg2;
};
void instance_unit_tests() {
    mempool *mp = new_mempool();

    struct cmd *c1 = mpalloc(mp, struct cmd);
    c1->instance_info = &cmd_info;
    c1->name = "add";
    c1->arg1 = 1;
    c1->arg2 = 2;

    struct cmd *c2 = mpalloc(mp, struct cmd);
    c2->instance_info = &cmd_info;
    c2->name = "add";
    c2->arg1 = 1;
    c2->arg2 = 2;

    struct cmd *c3 = mpalloc(mp, struct cmd);
    c3->instance_info = &cmd_info;
    c3->name = "sub";
    c3->arg1 = 8;
    c3->arg2 = 3;

    struct simple_cmd *sc1 = mpalloc(mp, struct simple_cmd);
    sc1->name = "mov";
    sc1->arg1 = 10;
    sc1->arg2 = 20;

    assert(has_instance_info(c1));
    assert(has_instance_info(c2));
    assert(has_instance_info(c3));
    assert(!has_instance_info(sc1));

    assert(strcmp(get_instance_name(c1), "cmd") == 0);
    assert(strcmp(get_instance_name(c2), "cmd") == 0);
    assert(get_instance_name(sc1) == NULL);

    assert(is_instance_named(c1, "cmd"));
    assert(!is_instance_named(c1, "command"));
    assert(!is_instance_named(sc1, "cmd"));
    assert(!is_instance_named(sc1, "simple_cmd"));

    assert(compare_instances(c1, c2) == 0);
    assert(compare_instances(c2, c3) < 0);

    assert(instances_equal(c1, c2));
    assert(!instances_equal(c1, c3));
    assert(!instances_equal(c1, sc1));

    assert(instance_matches(c1, "name", "add"));
    assert(instance_matches(c2, "name", "add"));
    assert(!instance_matches(c3, "name", "add"));
    assert(!instance_matches(sc1, "name", "add"));

    assert(instance_hash(c1) != 0);
    assert(instance_hash(c2) != 0);
    assert(instance_hash(c3) != 0);
    assert(instance_hash(sc1) == 0);

    assert(strcmp(instance_to_string(mp, c1), "add(1, 2)") == 0);
    assert(strcmp(instance_to_string(mp, c3), "sub(8, 3)") == 0);
    assert(instance_to_string(mp, sc1) == NULL);

    mempool_release(mp);
}
static bool cmd_equals(const void *a, const void *b) {
    if (a == NULL && b == NULL) return true;
    if (a == NULL && b != NULL) return false;
    if (b == NULL && a != NULL) return false;
    if (a == b) return true;
    struct cmd *c1 = (struct cmd *)a;
    struct cmd *c2 = (struct cmd *)b;

    if (strcmp(c1->name, c2->name) != 0) return false;
    if (c1->arg1 != c2->arg1) return false;
    if (c1->arg2 != c2->arg2) return false;
    return true;
}
static bool cmd_matches(const void *item, const char *match_type, const void *match_criteria) {
    if (item == NULL) return false;
    struct cmd *c = (struct cmd *)item;

    if (strcmp(match_type, "name") == 0) {
        return strcmp(c->name, match_criteria) == 0;
    }
    return false;
}
static int cmd_compare(const void *a, const void *b) {
    if (a == NULL && b == NULL) return 0;
    if (a == NULL && b != NULL) return 1;
    if (b == NULL && a != NULL) return -1;
    if (a == b) return 0;
    struct cmd *c1 = (struct cmd *)a;
    struct cmd *c2 = (struct cmd *)b;
    
    int r;
    if ((r = strcmp(c1->name, c2->name)) != 0) return r;
    if ((r = (c1->arg1 - c2->arg1)) != 0) return r;
    if ((r = (c1->arg2 - c2->arg2)) != 0) return r;
    return 0;
}
static unsigned long cmd_hash(const void *item) {
    if (item == NULL) return 0;

    char buffer[64];
    struct cmd *c = (struct cmd *)item;
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s-%d-%d", c->name, c->arg1, c->arg2);
    return hash(buffer);
}
static char *cmd_to_string(mempool *mp, const void *item) {
    if (item == NULL) return NULL;
    struct cmd *c = (struct cmd *)item;
    char *buff = mpallocn(mp, 128, "to_string");
    memset(buff, 0, 128);
    snprintf(buff, 128 - 1, "%s(%d, %d)", c->name, c->arg1, c->arg2);
    return buff;
}
#endif
