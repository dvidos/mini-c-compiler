#include "unit_tests.h"
#include "mempool.h"
#include <string.h>

/*
    // usage:
    instance_info str_info_data = {
        .magic_number = STRUCT_INFO_MAGIC_NUMBER,
        .name = "str",
        .equals = str_equals,
        .matches = str_matches,
        .compare = str_compare,
        .hash = str_hash,
        .to_string = str_to_string
    };
    instance_info *str_info = &str_info_data;

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

#define STRUCT_INFO_MAGIC_NUMBER   0xB6F5536C  // from random.org

typedef struct instance_info {
    int magic_number;    // to verify existence of this structure
    const char *name;   // e.g. "token", same with what is given to mpalloc()

    bool (*equals)(const void *a, const void *b);
    bool (*matches)(const void *item, const char *match_type, const void *match_criteria);
    int (*compare)(const void *a, void *b);
    unsigned long (*hash)(const void *item);
    char *(*to_string)(mempool *mp, const void *item);
} instance_info;


bool has_instance_info(const void *ptr);
const char *get_instance_name(const void *ptr);
bool is_instance_named(const void *ptr, const char *name);
int compare_instances(const void *a, const void *b);
bool instances_equal(const void *a, const void *b);
bool instance_matches(const void *item, const char *match_type, const void *match_criteria);
unsigned long instance_hash(const void *item);
const char *instance_to_string(mempool *mp, const void *item);

