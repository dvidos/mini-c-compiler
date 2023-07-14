#pragma once


typedef struct str str;


typedef int comparator_func(const void *a, const void *b);
typedef bool filterer_func(void *item);
typedef void *mapper_func(void *item, mempool *mp);
typedef void *reducer_func(void *item, void *prev_value, mempool *mp);
typedef str *classifier_func(void *item);


