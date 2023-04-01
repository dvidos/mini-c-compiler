#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "str.h"


static void _free(str *s);
static void _ensure_capacity_for(str *s, int extra_space_needed);
static void _clear(str *s);
static void _add(str *s, str *other);
static void _adds(str *s, char *strz);
static void _addc(str *s, char c);
static void _addf(str *s, char *fmt, ...);
static void _padr(str *s, int len, char c);


struct str_vtable vtable = {
    .clear = _clear,
    .add = _add,
    .adds = _adds,
    .addc = _addc,
    .addf = _addf,
    .padr = _padr,
    .free = _free
};

str *new_str() {
    str *s = malloc(sizeof(str));
    s->capacity = 10;
    s->buffer = malloc(s->capacity);
    s->length = 0;
    s->buffer[s->length] = '\0';

    s->v = &vtable;
    return s;
}

str *new_str_from(char *init_value) {
    str *s = new_str();
    _adds(s, init_value);
    return s;
}

static void _free(str *s) {
    if (s->buffer != NULL)
        free(s->buffer);
    free(s);
}

static void _ensure_capacity_for(str *s, int extra_space_needed) {
    if (s->capacity < s->length + extra_space_needed + 1) {
        while (s->capacity < s->length + extra_space_needed + 1)
            s->capacity *= 2;
        s->buffer = realloc(s->buffer, s->capacity);
    }
}

static void _clear(str *s) {
    s->length = 0;
    s->buffer[s->length] = 0;
}

static void _add(str *s, str *other) {
    if (other == NULL)
        return;
    
    _ensure_capacity_for(s, other->length);
    strcpy(s->buffer + s->length, other->buffer);
    s->length += other->length;
}

static void _adds(str *s, char *strz) {
    if (strz == NULL)
        return;
    
    _ensure_capacity_for(s, strlen(strz));
    strcpy(s->buffer + s->length, strz);
    s->length += strlen(strz);
}

static void _addc(str *s, char c) {
    if (c == 0)
        return;

    _ensure_capacity_for(s, 1);
    s->buffer[s->length++] = c;
    s->buffer[s->length] = 0;
}

static void _addf(str *s, char *fmt, ...) {
    // we don't know how many bytes we'll need....
    char buff[128];

    va_list vl;
    va_start(vl, fmt);
    vsnprintf(buff, sizeof(buff) - 1, fmt, vl);
    va_end(vl);
    buff[sizeof(buff) - 1] = 0;

    _adds(s, buff);
}

static void _padr(str *s, int len, char c) {
    _ensure_capacity_for(s, len - s->length + 1);
    while (s->length < len)
        s->buffer[s->length++] = c;
    s->buffer[s->length] = 0;
}
