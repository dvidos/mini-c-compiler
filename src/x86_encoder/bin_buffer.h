#pragma once
#include <stdint.h>


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct bin_buffer {
    char *data;
    int capacity;
    int length;
    void (*clear)(struct bin_buffer *buff);
    void (*append)(struct bin_buffer *buff, struct bin_buffer *source);
    void (*add_byte)(struct bin_buffer *buff, u8 value);
    void (*add_bytes2)(struct bin_buffer *buff, u8 byte1, u8 byte2);
    void (*add_bytes3)(struct bin_buffer *buff, u8 byte1, u8 byte2, u8 byte3);
    void (*add_word)(struct bin_buffer *buff, u16 value);
    void (*add_dword)(struct bin_buffer *buff, u32 value);
    void (*add_quad)(struct bin_buffer *buff, u64 value);
    void (*add_repeat_bytes)(struct bin_buffer *buff, u8 value, int times);
    void (*add_repeat_words)(struct bin_buffer *buff, u16 value, int times);
    void (*add_repeat_dwords)(struct bin_buffer *buff, u32 value, int times);
    void (*add_repeat_quads)(struct bin_buffer *buff, u64 value, int times);
    void (*free)(struct bin_buffer *buff);
};

struct bin_buffer *new_bin_buffer();
