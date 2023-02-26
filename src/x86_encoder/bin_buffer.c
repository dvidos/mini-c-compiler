#include "bin_buffer.h"
#include <stdlib.h>
#include <string.h>


static void bin_buff_clear(struct bin_buffer *buff);
static void bin_buff_append(struct bin_buffer *buff, struct bin_buffer *source);
static void bin_buff_add_byte(struct bin_buffer *buff, u8 value);
static void bin_buff_add_bytes2(struct bin_buffer *buff, u8 byte1, u8 byte2);
static void bin_buff_add_bytes3(struct bin_buffer *buff, u8 byte1, u8 byte2, u8 byte3);
static void bin_buff_add_word(struct bin_buffer *buff, u16 value);
static void bin_buff_add_dword(struct bin_buffer *buff, u32 value);
static void bin_buff_add_quad(struct bin_buffer *buff, u64 value);
static void bin_buff_add_repeat_bytes(struct bin_buffer *buff, u8 value, int times);
static void bin_buff_add_repeat_words(struct bin_buffer *buff, u16 value, int times);
static void bin_buff_add_repeat_dwords(struct bin_buffer *buff, u32 value, int times);
static void bin_buff_add_repeat_quads(struct bin_buffer *buff, u64 value, int times);
static void bin_buff_free(struct bin_buffer *buff);


struct bin_buffer *new_bin_buffer() {
    struct bin_buffer *p = malloc(sizeof(struct bin_buffer));
    p->capacity = 10;
    p->data = malloc(p->capacity);
    p->length = 0;

    p->clear = bin_buff_clear;
    p->append = bin_buff_append;
    p->add_byte = bin_buff_add_byte;
    p->add_bytes2 = bin_buff_add_bytes2;
    p->add_bytes3 = bin_buff_add_bytes3;
    p->add_word = bin_buff_add_word;
    p->add_dword = bin_buff_add_dword;
    p->add_quad = bin_buff_add_quad;
    p->add_repeat_bytes = bin_buff_add_repeat_bytes;
    p->add_repeat_words = bin_buff_add_repeat_words;
    p->add_repeat_dwords = bin_buff_add_repeat_dwords;
    p->add_repeat_quads = bin_buff_add_repeat_quads;
    p->free = bin_buff_free;

    return p;
}



static void bin_buff_clear(struct bin_buffer *buff) {
    buff->length = 0;
}

static void bin_buff_append(struct bin_buffer *buff, struct bin_buffer *source) {
    if (buff->length + source->length >= buff->capacity) {
        buff->capacity *= 2;
        buff->data = realloc(buff->data, buff->capacity);
    }
    memcpy(buff->data + buff->length, source->data, source->length);
    buff->length += source->length;
}

static void bin_buff_add_byte(struct bin_buffer *buff, u8 value) {
    if (buff->length + 1 >= buff->capacity) {
        buff->capacity *= 2;
        buff->data = realloc(buff->data, buff->capacity);
    }
    buff->data[buff->length] = value;
    buff->length++;
}

static void bin_buff_add_bytes2(struct bin_buffer *buff, u8 byte1, u8 byte2) {
    bin_buff_add_byte(buff, byte1);
    bin_buff_add_byte(buff, byte2);
}

static void bin_buff_add_bytes3(struct bin_buffer *buff, u8 byte1, u8 byte2, u8 byte3) {
    bin_buff_add_byte(buff, byte1);
    bin_buff_add_byte(buff, byte2);
    bin_buff_add_byte(buff, byte3);
}

static void bin_buff_add_word(struct bin_buffer *buff, u16 value) {
    if (buff->length + sizeof(value) >= buff->capacity) {
        buff->capacity *= 2;
        buff->data = realloc(buff->data, buff->capacity);
    }
    u16 *p = (u16 *)(&buff->data[buff->length]);
    *p = value;
    buff->length += sizeof(value);
}

static void bin_buff_add_dword(struct bin_buffer *buff, u32 value) {
    if (buff->length + sizeof(value) >= buff->capacity) {
        buff->capacity *= 2;
        buff->data = realloc(buff->data, buff->capacity);
    }
    u32 *p = (u32 *)(&buff->data[buff->length]);
    *p = value;
    buff->length += sizeof(value);
}

static void bin_buff_add_quad(struct bin_buffer *buff, u64 value) {
    if (buff->length + sizeof(value) >= buff->capacity) {
        buff->capacity *= 2;
        buff->data = realloc(buff->data, buff->capacity);
    }
    u64 *p = (u64 *)(&buff->data[buff->length]);
    *p = value;
    buff->length += sizeof(value);
}

static void bin_buff_add_repeat_bytes(struct bin_buffer *buff, u8 value, int times) {
    while (times--)
        bin_buff_add_byte(buff, value);
}

static void bin_buff_add_repeat_words(struct bin_buffer *buff, u16 value, int times) {
    while (times--)
        bin_buff_add_word(buff, value);
}

static void bin_buff_add_repeat_dwords(struct bin_buffer *buff, u32 value, int times) {
    while (times--)
        bin_buff_add_dword(buff, value);
}

static void bin_buff_add_repeat_quads(struct bin_buffer *buff, u64 value, int times) {
    while (times--)
        bin_buff_add_quad(buff, value);
}

static void bin_buff_free(struct bin_buffer *buff) {
    free(buff->data);
    free(buff);
}

