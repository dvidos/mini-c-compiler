#include "../unit_tests.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "string.h"


static void _free(string *s);
static void _ensure_capacity_for(string *s, int extra_space_needed);
static void _clear(string *s);
static void _add(string *s, string *other);
static void _adds(string *s, char *strz);
static void _addc(string *s, char c);
static void _addf(string *s, char *fmt, ...);
static void _padr(string *s, int len, char c);
static void _add_escaped(string *s, char *str);


struct string_vtable vtable = {
    .clear = _clear,
    .add = _add,
    .adds = _adds,
    .addc = _addc,
    .addf = _addf,
    .add_escaped = _add_escaped,
    .padr = _padr,
    .free = _free
};

string *new_string() {
    string *s = malloc(sizeof(string));
    s->capacity = 10;
    s->buffer = malloc(s->capacity);
    s->length = 0;
    s->buffer[s->length] = '\0';

    s->v = &vtable;
    return s;
}

string *new_string_from(char *init_value) {
    string *s = new_string();
    _adds(s, init_value);
    return s;
}

static void _free(string *s) {
    if (s->buffer != NULL)
        free(s->buffer);
    free(s);
}

static void _ensure_capacity_for(string *s, int extra_space_needed) {
    if (s->capacity < s->length + extra_space_needed + 1) {
        while (s->capacity < s->length + extra_space_needed + 1)
            s->capacity *= 2;
        s->buffer = realloc(s->buffer, s->capacity);
    }
}

static void _clear(string *s) {
    s->length = 0;
    s->buffer[s->length] = 0;
}

static void _add(string *s, string *other) {
    if (other == NULL)
        return;
    
    _ensure_capacity_for(s, other->length);
    strcpy(s->buffer + s->length, other->buffer);
    s->length += other->length;
}

static void _adds(string *s, char *strz) {
    if (strz == NULL)
        return;
    
    _ensure_capacity_for(s, strlen(strz));
    strcpy(s->buffer + s->length, strz);
    s->length += strlen(strz);
}

static void _addc(string *s, char c) {
    if (c == 0)
        return;

    _ensure_capacity_for(s, 1);
    s->buffer[s->length++] = c;
    s->buffer[s->length] = 0;
}

static void _addf(string *s, char *fmt, ...) {
    // we don't know how many bytes we'll need....
    char buff[128];

    va_list vl;
    va_start(vl, fmt);
    vsnprintf(buff, sizeof(buff) - 1, fmt, vl);
    va_end(vl);
    buff[sizeof(buff) - 1] = 0;

    _adds(s, buff);
}

static void _add_escaped(string *s, char *str) {
    while (*str != 0) {
        switch (*str) {
            case '\n': _adds(s, "\\n"); break;
            case '\r': _adds(s, "\\r"); break;
            case '\t': _adds(s, "\\t"); break;
            case '\\': _adds(s, "\\\\"); break;
            default: _addc(s, *str);
        }
        str++;
    }
}

static void _padr(string *s, int len, char c) {
    _ensure_capacity_for(s, len - s->length + 1);
    while (s->length < len)
        s->buffer[s->length++] = c;
    s->buffer[s->length] = 0;
}


#ifdef INCLUDE_UNIT_TESTS

void string_unit_tests() {
    assert(1 == 1);
}

#endif // UNIT_TESTS
