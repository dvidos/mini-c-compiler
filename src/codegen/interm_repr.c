#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
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


typedef struct data_chunk {
    char *mnemonic;
    int size;
    symbol *sym;
    int address;
    void *init_data;
    int init_data_size;
} data_chunk;

typedef struct data_seg {
    data_chunk *chunks[512];
    int used_chunks;
    int allocated_size;
} data_seg;



typedef enum code_chunk_type {
    CCT_JMP,
    CCT_TAC,
    CCT_STRING,
} code_chunk_type;

typedef struct code_chunk {
    char *label;
    enum code_chunk_type type;
    union {
        struct three_address_code {
            int operator;
            // so far, each operator can be a symbol, a number, a register,
            // or the address pointed by a register
            // as an lvalue, it can be either a symbol, or an address pointed by a register.
            // in assembly, a symbol represents the address of the symbol, 
            //              and brackets (e.g. [my_str]) represent the value stored in that address
            int addr1; 
            int addr2;
            int addr3;
        } tac;
        struct jump_instruction {
            bool conditional;
            char *target_label;
        } jmp;
        char *str;
    } u;
} code_chunk;

struct code_seg {
    code_chunk *chunks[512];
    int used_chunks;
} code_seg;


char *next_code_label;
struct data_seg init_data; // data
struct data_seg zero_data; // bss
struct code_seg text_seg; // text



void init_intermediate_representation() {
    memset(&init_data, 0, sizeof(init_data));
    memset(&zero_data, 0, sizeof(zero_data));
    memset(&text_seg, 0, sizeof(text_seg));
    next_code_label = NULL;
}

// --------------------------------------------------------------------

#define WHILES_STACK_SIZE  16

static int regs_counter = 1; 
static int ifs_counter = 0;
static int whiles_counter = 0;
static int whiles_stack_len;
static int whiles_stack[WHILES_STACK_SIZE];

int get_next_reg_num() {
    return ++regs_counter;
}

int get_next_if_num() {
    return ++ifs_counter;
}

// --------------------------------------------------------------------

void push_while() {
    if (whiles_stack_len == WHILES_STACK_SIZE) {
        error(NULL, 0, "more than %d nested whiles detected, we need more stack!", WHILES_STACK_SIZE);
        return;
    }
    whiles_stack[whiles_stack_len++] = ++whiles_counter;
}

int get_curr_while_num() {
    if (whiles_stack_len == 0) {
        error(NULL, 0, "while was expected, but none started");
        return 0;
    }
    return whiles_stack[whiles_stack_len - 1];
}

void pop_while() {
    if (whiles_stack_len == 0) {
        error(NULL, 0, "stack of whiles underflow!");
        return;
    }
    whiles_stack_len--;
}


// -----------------------------------

void reserve_data_area(char *name, int size, bool initialized, void *initial_data) {
    // create a new mem chunk and append it to the list
    data_chunk *chunk = malloc(sizeof(data_chunk));
    chunk->size = size;
    chunk->mnemonic = strdup(name);
    chunk->init_data = initial_data;
    chunk->init_data_size = size;
    
    data_seg *seg = initialized ? &init_data : &zero_data;
    seg->chunks[seg->used_chunks++] = chunk;
    seg->allocated_size += size;
}

void add_jump_operation(char *label, bool conditional, char *target_label) {
    // either conditional or unconditional branch
    // we'll figure out the CMP operation, later
    code_chunk *c = malloc(sizeof(code_chunk));
    c->type = CCT_JMP;
    c->label = (label == NULL) ? NULL : strdup(label);
    c->u.jmp.conditional = conditional;
    c->u.jmp.target_label = strdup(target_label);

    text_seg.chunks[text_seg.used_chunks++] = c;
}

void add_output_operation(char *label, int op, int address1, int address2, int address3) {
    // essentially a Three Address Code.
    code_chunk *c = malloc(sizeof(code_chunk));
    c->type = CCT_TAC;
    c->label = (label == NULL) ? NULL : strdup(label);
    c->u.tac.operator = op;
    c->u.tac.addr1 = address1;
    c->u.tac.addr2 = address2;
    c->u.tac.addr3 = address3;

    text_seg.chunks[text_seg.used_chunks++] = c;
}

// a = b <op> c
// a = <op> b
// a = b
// if a <rel-op> b goto x
// goto x
// a = call b using d, e, f

typedef enum interm_repr_operator {
    IR_NOP,
    IR_IADD,   // a = b + c
    IR_ISUB,   // a = b - c
    IR_IMUL,   // a = b * c
    IR_IDIV,   // a = b / c
    IR_INV,    // a = ~b (bitwise inverse)
    IR_ASSIGN, // a = b
    IR_JMP_EQ, // GOTO a if b == c
    IR_JMP_NE, // GOTO a if b != c
    IR_JMP_LE, // GOTO a if b <= c
    IR_JMP_LT, // GOTO a if b < c
    IR_JMP_GE, // GOTO a if b >= c
    IR_JMP_GT, // GOTO a if b > c
    IR_JMP_Z,  // GOTO a if b == 0
    IR_JMP_NZ, // GOTO a if b != 0
    IR_JMP,    // GOTO a
    IR_CALL,   // push a & goto a
    IR_RET,    // goto (popped value)
    IR_PUSH,   // push a
    IR_POP,    // pop a
} interm_repr_operator;


void ir_set_next_label(char *fmt, ...) {
    char buff[100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, 100, fmt, args);
    va_end(args);
    next_code_label = strdup(buff);
}

void ir_jmp(char *label_fmt, ...) {
    char label[100];
    va_list args;
    va_start(args, label_fmt);
    vsnprintf(label, 100, label_fmt, args);
    va_end(args);

    ir_add_str("JMP %s", label);
}

void ir_add_str(char *fmt, ...) {
    char buff[100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, 100, fmt, args);
    va_end(args);

    code_chunk *c = malloc(sizeof(code_chunk));
    c->type = CCT_STRING;
    c->label = next_code_label; // it may be null
    c->u.str = strdup(buff);

    text_seg.chunks[text_seg.used_chunks++] = c;
    next_code_label = NULL;
}

void ir_dump_code_segment() {
    for (int i = 0; i < sizeof(text_seg.chunks) / sizeof(text_seg.chunks[0]); i++) {
        code_chunk *c = text_seg.chunks[i];
        if (c == NULL)
            break;

        if (c->label != NULL)
            printf("%s:\n", c->label);

        printf("\t");
        if (c->type == CCT_JMP) {
            if (c->u.jmp.conditional) {
                printf("JMP <condition> %s", c->u.jmp.target_label);
            } else {
                printf("JMP %s", c->u.jmp.target_label);
            }
        } else if (c->type == CCT_TAC) {
            // we need to print the operator and the three addresses
            printf("OP <addr1> <addr2> <addr3>");
        } else if (c->type == CCT_STRING) {
            printf("%s", c->u.str);
        }
        printf("\n");
    }
}

void ir_dump_data_segment(bool initialized) {
    data_seg *seg = initialized ? &init_data : &zero_data;
    for (int i = 0; i < sizeof(init_data.chunks) / sizeof(init_data.chunks[0]); i++) {
        data_chunk *c = seg->chunks[i];
        if (c == NULL)
            break;

        printf("\t%-15s %5d\n", c->mnemonic, c->size);
    }
}

