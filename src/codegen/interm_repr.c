#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "interm_repr.h"
#include "../err_handler.h"
#include "../symbol.h"

/*
    It seems there are lots of formats for intermediate representation (IR)

    Some examples here:
    https://suif.stanford.edu/dragonbook/lecture-notes/Stanford-CS143/16-Intermediate-Rep.pdf

    Wikipedia also has good article:
    https://en.wikipedia.org/wiki/Intermediate_representation


    One central quation on the level of IR, is how low we want to get.
    For example, array subscripts and struct memberships can be either
    left for further transformation, or be replaced by offsets

    Usually we model things for a machine that has infinite number of registers
    and we track which registers are used at the time.

    Expressions can be translated into assembly, by:

    - using a post-order tranversal,
    - each operation is told which register to store the result,
    - results from previous operations are used in the subsequent ones,
    - finally the result of the whole expression is stored on a predefined register

    Branching is more interesting, in the sense that,
    blocks without jumps are identified and can be moved around.

    Wikipedia says that a popular format is Three Address Code, i.e.
    a format in the form of "a1 = a2 <op> a3". 

    Ideally, our IR will contain no ambiguous operations,
    so operations can be translated into instructions.
    examples are: IMUL, FMUL, ISUB, etc, so that 
    types are specific.

    Oh, the other thing is that IR is mainly produced for 
    optimization reasons, one can generate code straight 
    out of the AST or a DAG.
*/

static void ir_init();
static void ir_set_next_label(char *fmt, ...);
static void ir_add_str(char *fmt, ...);
static void ir_add_comment(char *fmt, ...);
static void ir_jmp(char *label_fmt, ...);
static int  ir_reserve_data(int bytes, void *init_data);
static int  ir_reserve_strz(char *str);
static void ir_add_symbol(char *name, bool is_func, int offset);
static void ir_dump_symbols();
static void ir_dump_code_segment();
static void ir_dump_data_segment();

intermediate_representation_ops ir = {
    .init = ir_init,
    .set_next_label = ir_set_next_label,
    .add_str = ir_add_str,
    .add_comment = ir_add_comment,
    .jmp = ir_jmp,
    .reserve_data = ir_reserve_data,
    .reserve_strz = ir_reserve_strz,
    .add_symbol = ir_add_symbol,
    .dump_symbols = ir_dump_symbols,
    .dump_code_segment = ir_dump_code_segment,
    .dump_data_segment = ir_dump_data_segment
};

// --------------------------------------------

struct code_chunk {
    char *label;
    bool is_comment;
    char *code;
};

struct data_chunk {
    int offset;
    int size;
    char *init_data;
};

struct ir_symbol {
    char *name;
    bool is_func;
    int data_offset;
};

struct code_chunk **code_seg;
struct data_chunk **data_seg;
struct ir_symbol **symbols_table;

static int code_seg_capacity;
static int data_seg_capacity;
static int symbols_table_capacity;

static int code_seg_entries;
static int data_seg_entries;
static int data_seg_offset;
static int symbols_table_entries;


static char *next_code_label;

// typedef struct data_chunk {
//     char *mnemonic;
//     int size;
//     symbol *sym;
//     int address;
//     void *init_data;
//     int init_data_size;
// } data_chunk;

// typedef struct data_seg {
//     data_chunk *chunks[512];
//     int used_chunks;
//     int allocated_size;
// } data_seg;

// typedef enum code_chunk_type {
//     CCT_JMP,
//     CCT_TAC,
//     CCT_STRING,
//     CCT_COMMENT,
// } code_chunk_type;

// typedef struct code_chunk {
//     char *label;
//     enum code_chunk_type type;
//     union {
//         struct three_address_code {
//             int operator;
//             // so far, each operator can be a symbol, a number, a register,
//             // or the address pointed by a register
//             // as an lvalue, it can be either a symbol, or an address pointed by a register.
//             // in assembly, a symbol represents the address of the symbol, 
//             //              and brackets (e.g. [my_str]) represent the value stored in that address
//             int addr1; 
//             int addr2;
//             int addr3;
//         } tac;
//         struct jump_instruction {
//             bool conditional;
//             char *target_label;
//         } jmp;
//         char *str;
//     } u;
// } code_chunk;

// struct code_seg {
//     code_chunk *chunks[512];
//     int used_chunks;
// } code_seg;


// char *next_code_label;
// struct data_seg init_data; // data
// struct data_seg zero_data; // bss
// struct code_seg text_seg; // text

// #define STRINGS_TABLE_SIZE   1024
// char *strings_table;
// int strings_table_len = 0;




static void ir_init() {
    code_seg_capacity = 512;
    data_seg_capacity = 128;
    symbols_table_capacity = 128;

    code_seg = malloc(sizeof(struct code_chunk) * code_seg_capacity);
    data_seg = malloc(sizeof(struct code_chunk) * data_seg_capacity);
    symbols_table = malloc(sizeof(struct symbol) * symbols_table_capacity);

    code_seg_entries = 0;
    data_seg_entries = 0;
    data_seg_offset = 0;
    symbols_table_entries = 0;

    next_code_label = NULL;

    // memset(&init_data, 0, sizeof(init_data));
    // memset(&zero_data, 0, sizeof(zero_data));
    // memset(&text_seg, 0, sizeof(text_seg));
    // next_code_label = NULL;

    // strings_table = malloc(STRINGS_TABLE_SIZE);
    // memset(strings_table, 0, STRINGS_TABLE_SIZE);
    // strings_table_len = 1; // allow first char to be the empty string
}

static void ir_set_next_label(char *fmt, ...) {
    char buff[100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, 100, fmt, args);
    va_end(args);
    next_code_label = strdup(buff);
}

static void ir_add_comment(char *fmt, ...) {
    char buff[100];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
    va_end(args);

    struct code_chunk *c = malloc(sizeof(struct code_chunk));
    c->label = NULL;
    c->is_comment = true;
    c->code = strdup(buff);

    if (code_seg_entries == code_seg_capacity) {
        code_seg_capacity *= 2;
        code_seg = realloc(code_seg, code_seg_capacity);
    }
    code_seg[code_seg_entries++] = c;
}

static void ir_add_str(char *fmt, ...) {
    char buff[100];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
    va_end(args);

    struct code_chunk *c = malloc(sizeof(struct code_chunk));
    c->label = next_code_label;
    c->is_comment = false;
    c->code = strdup(buff);
    next_code_label = NULL;

    if (code_seg_entries == code_seg_capacity) {
        code_seg_capacity *= 2;
        code_seg = realloc(code_seg, code_seg_capacity);
    }
    code_seg[code_seg_entries++] = c;
}

static void ir_jmp(char *label_fmt, ...) {
    char label[100];
    va_list args;

    va_start(args, label_fmt);
    vsnprintf(label, sizeof(label), label_fmt, args);
    va_end(args);

    ir_add_str("JMP %s", label);
}

static int ir_reserve_data(int bytes, void *init_data) {
    if (bytes == 0)
        return 0;
    
    struct data_chunk *d = malloc(sizeof(struct data_chunk));
    d->offset = data_seg_offset;
    d->size = bytes;
    d->init_data = init_data;

    if (data_seg_entries == data_seg_capacity) {
        data_seg_capacity *= 2;
        data_seg = realloc(data_seg, data_seg_capacity);
    }
    data_seg[data_seg_entries++] = d;
    data_seg_offset += d->size;

    return d->offset;
}

static int ir_reserve_strz(char *str) {
    return ir_reserve_data(strlen(str) + 1, str);
}

static void ir_add_symbol(char *name, bool is_func, int offset) {
    struct ir_symbol *s = malloc(sizeof(struct ir_symbol));
    s->name = name;
    s->is_func = is_func;
    s->data_offset = offset;

    if (symbols_table_entries == symbols_table_capacity) {
        symbols_table_capacity *= 2;
        symbols_table = realloc(symbols_table, symbols_table_capacity);
    }
    symbols_table[symbols_table_entries++] = s;
}

static void ir_dump_symbols() {
    printf("    # Name                 Type  Data offs\n");
    //        123 12345678901234567890 1234  123456789
    for (int i = 0; i < symbols_table_entries; i++) {
        struct ir_symbol *s = symbols_table[i];
        printf("  %3d %-20s %-4s  %9d\n", 
            i,
            s->name,
            s->is_func ? "FUNC" : "VAR",
            s->is_func ? 0 : s->data_offset
        );
    }
}

static void ir_dump_code_segment() {
    for (int i = 0; i < code_seg_entries; i++) {
        struct code_chunk *c = code_seg[i];
        if (c->label != NULL)
            printf("%s:\n", c->label);

        printf("%s%s\n", c->is_comment ? "" : "\t", c->code);
    }
}

static void pretty_print_string(char *str) {
    char *buff = malloc(strlen(str) * 2);
    char *dest = buff;
    
    while (*str != 0) {
        if      (*str == '\n') { *dest++ = '\\'; *dest++ = 'n'; }
        else if (*str == '\r') { *dest++ = '\\'; *dest++ = 'r'; }
        else if (*str == '\t') { *dest++ = '\\'; *dest++ = 't'; }
        else { *dest++ = *str; }
        str++;
    }
    *dest = *str; // zero terminator
    printf("%s", buff);
    free(buff);
}

static void ir_dump_data_segment() {
    printf("    #    Offset      Size  Initial value\n");
    //        123 123456789 123456789  12 12 12 12...
    for (int i = 0; i < data_seg_entries; i++) {
        struct data_chunk *d = data_seg[i];
        printf("  %3d %9d %9d  ", i, d->offset, d->size);

        if (d->init_data == NULL) {
            printf("\n");
            continue;
        }

        bool is_string = d->init_data != NULL 
            && d->init_data[0] > ' ' && d->init_data[0] <= 'z'
            && d->init_data[d->size - 1] == 0;
        if (is_string) {
            printf("\"");
            pretty_print_string(d->init_data);
            printf("\"\n");
            continue;
        }

        for (int j = 0; j < d->size; j++)
            printf("%02x ", (unsigned char)d->init_data[j]);
        printf("\n");
    }
}

