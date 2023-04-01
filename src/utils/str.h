#pragma once

struct str_vtable;

typedef struct str {
    char *buffer;
    int length;   // does not include zero terminator
    int capacity; // malloc'ed size of buffer

    struct str_vtable *v;
} str;

str *new_str();
str *new_str_from(char *init_value);

struct str_vtable {
    void (*clear)(str *s);
    void (*add)(str *s, str *other);
    void (*addz)(str *s, char *strz);
    void (*addc)(str *s, char c);
    void (*addf)(str *s, char *fmt, ...);
    
    // void (*prepend)(str *s, str *other);
    // void (*repeat)(str *s, char c, int times);

    // void (*format)(str *s, char *fmt, ...);
    // int (*index_of)(str *s, char c);
    // char (*char_at)(str *s, int index);
    // int (*find_strz)(str *haystack, char *needle);
    // int (*find_str)(str *haystack, str *needle);
    // str *(*substr)(str *s, int index, int length);
    // str *(*left)(str *s, int length);
    // str *(*right)(str *s, int length);
    // str *(*trim)(str *s, char *characters);

    // str *(*insert_at)(str *s, int index, str *other);

    // int (*cmp)(str *s, str *other);
    // bool (*equals)(str *s, str *other);
    // unsigned long (*hash)(str *s);
    // array *(*split)(str *s, char *delimiters);

    void (*free)(str *s);
};
