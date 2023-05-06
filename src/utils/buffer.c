#include "../unit_tests.h"
#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static void _clear(buffer *buff);
static void _append(buffer *buff, buffer *source);
static void _add_byte(buffer *buff, u8 value);
static void _add_word(buffer *buff, u16 value);
static void _add_dword(buffer *buff, u32 value);
static void _add_quad(buffer *buff, u64 value);
static void _add_strz(buffer *buff, char *strz);
static void _add_mem(buffer *buff, void *mem, int len);
static void _add_zeros(buffer *buff, int len);
static void _fill(buffer *buff, int target_length, u8 filler);
static void _round_up(buffer *buff, int round_value, u8 filler);
static bool _from_file(buffer *buff, FILE *f, size_t bytes);
static bool _to_file(buffer *buff, FILE *f);
static void _free(buffer *buff);


buffer *new_buffer() {
    buffer *p = malloc(sizeof(buffer));
    p->capacity = 10;
    p->buffer = malloc(p->capacity);
    p->length = 0;

    p->clear = _clear;
    p->append = _append;
    p->add_byte = _add_byte;
    p->add_word = _add_word;
    p->add_dword = _add_dword;
    p->add_quad = _add_quad;
    p->add_strz = _add_strz;
    p->add_mem = _add_mem;
    p->add_zeros = _add_zeros;
    p->fill = _fill;
    p->round_up = _round_up;

    p->from_file = _from_file;
    p->to_file = _to_file;

    p->free = _free;

    return p;
}



static void _clear(buffer *buff) {
    buff->length = 0;
}

static void _ensure_enough_capacity(buffer *buff, int space_needed) {
    if (buff->length + space_needed >= buff->capacity) {
        while (buff->length + space_needed >= buff->capacity)
            buff->capacity *= 2;

        buff->buffer = realloc(buff->buffer, buff->capacity);
    }
}


static void _append(buffer *buff, buffer *source) {
    _ensure_enough_capacity(buff, source->length);
    memcpy(buff->buffer + buff->length, source->buffer, source->length);
    buff->length += source->length;
}

static void _add_byte(buffer *buff, u8 value) {
    _ensure_enough_capacity(buff, 1);
    buff->buffer[buff->length] = value;
    buff->length++;
}

static void _add_word(buffer *buff, u16 value) {
    _ensure_enough_capacity(buff, 2);
    u16 *p = (u16 *)(&buff->buffer[buff->length]);
    *p = value;
    buff->length += sizeof(value);
}

static void _add_dword(buffer *buff, u32 value) {
    _ensure_enough_capacity(buff, 4);
    u32 *p = (u32 *)(&buff->buffer[buff->length]);
    *p = value;
    buff->length += sizeof(value);
}

static void _add_quad(buffer *buff, u64 value) {
    _ensure_enough_capacity(buff, 4);
    u64 *p = (u64 *)(&buff->buffer[buff->length]);
    *p = value;
    buff->length += sizeof(value);
}

static void _fill(buffer *buff, int target_length, u8 filler) {
    if (target_length <= buff->length)
        return;

    _ensure_enough_capacity(buff, target_length);
    int expansion = target_length - buff->length;
    memset(buff->buffer + buff->length, filler, expansion);
    buff->length += expansion;
}

static void _round_up(buffer *buff, int round_value, u8 filler) {
    if (buff->length == 0)
        return;
    
    int target_length = (((buff->length + round_value - 1) / round_value) * round_value);
    if (target_length == buff->length)
        return;

    _fill(buff, target_length, filler);
}

static void _add_mem(buffer *buff, void *mem, int len) {
    _ensure_enough_capacity(buff, len);
    char *pos = &buff->buffer[buff->length];
    memcpy(pos, mem, len);
    buff->length += len;
}

static void _add_strz(buffer *buff, char *strz) {
    _add_mem(buff, strz, strlen(strz) + 1); // include null terminator
}

static void _add_zeros(buffer *buff, int len) {
    _ensure_enough_capacity(buff, len);
    char *pos = &buff->buffer[buff->length];
    memset(pos, 0, len);
    buff->length += len;
}

static bool _from_file(buffer *buff, FILE *f, size_t bytes) { 
    _ensure_enough_capacity(buff, buff->length + bytes);
    size_t gotten = fread(buff->buffer + buff->length, 1, bytes, f);
    buff->length += gotten;
    return gotten == bytes;
}

static bool _to_file(buffer *buff, FILE *f) { 
    size_t wrote = fwrite(buff->buffer, 1, (size_t)buff->length, f);
    return wrote == buff->length;
}

static void _free(buffer *buff) {
    free(buff->buffer);
    free(buff);
}


#ifdef INCLUDE_UNIT_TESTS

void buffer_unit_tests() {
    assert(1 == 1);
}

#endif // UNIT_TESTS
