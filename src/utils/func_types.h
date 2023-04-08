#pragma once


// the following functions to perform operations in collections
// - item is the item under operation, 
// - extra_data is any runtime value to affect the operation


// can perform one operation on an item
typedef void visitor_func(void *item, void *extra_data);

// can compare two items and return <0, 0, >0, similar to strcmp()
typedef int comparer_func(void *a, void *b);

// can provide a boolean value, whether an item passes a filter
typedef bool filter_func(void *item, void *extra_data);

// can check whether an item matches some criteria
typedef bool matcher_func(void *item, void *criteria, void *extra_data);

// based on an item, it can return something new
typedef void *mapper_func(void *item, void *extra_data);

// based on an item and a previous value, derives a new value
typedef void *reducer_func(void *item, void *prev_value, void *extra_data);

// allocates and returns a string with the description of an item. caller to free() the string
typedef char *to_string_func(void *item);

// returns a hash value based on the item
typedef long hasher_func(void *item);


