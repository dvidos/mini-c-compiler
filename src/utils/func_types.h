#pragma once

typedef void visitor_func(void *item);
typedef int comparer_func(void *a, void *b);
typedef bool filter_func(void *item);
typedef bool matcher_func(void *item, void *criteria);
typedef void *mapper_func(void *item);
typedef void *reducer_func(void *item, void *prev_value);
typedef char *to_string_func(void *item); // caller to free() the string
typedef long hasher_func(void *item);

