#pragma once

struct string_vtable;

typedef struct string {
    char *buffer;
    int length;   // does not include zero terminator
    int capacity; // malloc'ed size of buffer

    struct string_vtable *v;
} string;

string *new_string();
string *new_string_from(char *init_value);

struct string_vtable {
    void (*clear)(string *s);
    void (*add)(string *s, string *other);
    void (*adds)(string *s, char *str);
    void (*addc)(string *s, char c);
    void (*addf)(string *s, char *fmt, ...);
    void (*padr)(string *s, int length, char c);
    void (*add_escaped)(string *s, char *str); // converts "\n" into "\\n"
    
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

    void (*free)(string *s);
};
