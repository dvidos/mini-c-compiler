#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "str.h"


static void _free(str *s);
static void _ensure_capacity_for(str *s, int extra_space_needed);
static void _add(str *s, str *other);
static void _addz(str *s, char *strz);
static void _addc(str *s, char c);
static void _addf(str *s, char *fmt, ...);


struct str_vtable vtable = {
    .add = _add,
    .addz = _addz,
    .addc = _addc,
    .addf = _addf,
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
    _addz(s, init_value);
    return s;
}

static void _free(str *s) {
    if (s->buffer != NULL)
        free(s->buffer);
    free(s);
}

static void _ensure_capacity_for(str *s, int extra_space_needed) {
    if (s->length + extra_space_needed + 1 > s->capacity) {
        s->capacity = s->length + extra_space_needed + 1;
        s->buffer = realloc(s->buffer, s->capacity);
    }
}

static void _add(str *s, str *other) {
    if (other == NULL)
        return;
    
    _ensure_capacity_for(s, other->length);
    strcpy(s->buffer + s->length, other->buffer);
    s->length += other->length;
}

static void _addz(str *s, char *strz) {
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
    s->buffer[s->length] = c;
    s->length++;
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
    _addz(s, buff);
}
