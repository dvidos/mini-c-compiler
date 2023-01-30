#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../err_handler.h"
#include "../expression.h"
#include "../lexer/token.h"
#include "../statement.h"
#include "../symbol.h"
#include "../ast_node.h"



/*
    It seems there are lots of formats for intermediate representation (IR)

    Some examples here:
    https://suif.stanford.edu/dragonbook/lecture-notes/Stanford-CS143/16-Intermediate-Rep.pdf

    Wikipedia also has good article:
    https://en.wikipedia.org/wiki/Intermediate_representation

*/
/*
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


// we shall need some infrastructure
// reserve and release registers, a way to allocate things for the segments etc.


// ----------------------------------------------------------

typedef struct data_chunk {
    char *mnemonic;
    int size;
    symbol *sym;
    int address;

    bool non_zero_initialization; // determines if we go through data or bss segment.
    char *initialization_value;

    struct data_chunk *next;
} data_chunk;

typedef struct data_seg {
    struct data_chunk *chunks_head;
    struct data_chunk *chunks_tail;
    int allocated_size;
} data_seg;

struct data_seg init_data; // data
struct data_seg zero_data; // bss

data_chunk *allocate_data_chunk(int size, char *name, bool initialized) {
    // create a new mem chunk and append it to the list
    data_chunk *chunk = malloc(sizeof(data_chunk));
    chunk->size = size;
    chunk->mnemonic = name;
    chunk->next = NULL;
    
    data_seg *seg = initialized ? &init_data : &zero_data;
    chunk->address = seg->allocated_size;

    if (seg->chunks_tail == NULL) {
        seg->chunks_head = chunk;
        seg->chunks_tail = chunk;
    } else {
        seg->chunks_tail->next = chunk;
        seg->chunks_tail = chunk;
    }
    seg->allocated_size += chunk->size;

    return chunk;
}

void generate_data_segment_assembly(bool initialized_segment) {
    // walk the mem_chunks and print them out
    data_seg *seg = initialized_segment ? &init_data : &zero_data;
    data_chunk *chunk = seg->chunks_head;
    while (chunk != NULL) {
        printf("...");
        chunk = chunk->next;
    }
}

struct reference {
    // can be immediate number, a register name, a memory address, 
    // an address a register points, an address a register points with offset etc
    // if we can make this a value object will be best, to avoid allocation / deallocation
};

typedef struct code_chunk {
    int operator; // e.g. ADD, FADD, LOAD, STORE etc
    struct reference op1;
    struct reference op2;
} code_chunk;

#define CODE_CHUNKS_PER_PAGE  64   // to avoid allocating each individually
typedef struct code_seg_page { 
    struct code_chunk chunks[CODE_CHUNKS_PER_PAGE];
    int chunks_used; // 

    struct code_seg_page *next;
} code_seg_page;

struct code_seg {
    code_seg_page *pages_head;
    code_seg_page *pages_tail;
} code_seg;

void append_code_chunk(code_chunk *chunk) {
    code_seg_page *page;

    if (code_seg.pages_tail == NULL ||
        (code_seg.pages_tail != NULL && code_seg.pages_tail->chunks_used == CODE_CHUNKS_PER_PAGE)) {
        // we need a new page
        page = malloc(sizeof(code_seg_page));
        memset(page, 0, sizeof(code_seg_page));
        if (code_seg.pages_tail == NULL) {
            code_seg.pages_head = page;
            code_seg.pages_tail = page;
        } else {
            code_seg.pages_tail->next = page;
            code_seg.pages_tail = page;
        }
    }

    // append chunk to last page, given there is capacity
    page = code_seg.pages_tail;
    memcpy(&page->chunks[page->chunks_used], chunk, sizeof(code_chunk));
    page->chunks_used++;
}

void generate_code_segment_assembly() {
    // walk the pages and generate either mnemonic assembly
    // and/or machine code.
    // it can reference using mnemonics, that will resolved in the linking stages

    code_seg_page *page = code_seg.pages_head;
    while (page != NULL) {
        for (int i = 0; i < page->chunks_used; i++) {
            // ...
        }
        page = page->next;
    }
}

// --------------------------------------------------------

struct machine_model {
    int num_registers;
    char *registers_names[64]; // we could use HI/LO sections like AX, AH, AL, EAX, RAX
    int register_size; // 16, 32 or 64
};

// we could also have the mnemonicis or machine capabilities encoded somehow.

// ---------------------------------------------------------------------

void generate_expression_code(expression *expr) {
    // post-order visit, 
    // each operation is told where to store its result,
    // which is used 
}

void generate_statement_code(statement *stmt) {
    // blocks of statements and expressions, together with jumps
    switch (stmt->stmt_type) {
        case ST_BLOCK:
            break;

        case ST_VAR_DECL:
            // maybe assignment of value?
            // in theory, stack allocation has already happened
            break;

        case ST_IF:
            // generate condition and jumps in the true and false bodies
            break;

        case ST_WHILE:
            // generate condition and jumps in the end of the loop
            break;

        case ST_CONTINUE:
            // I think i'll handle it in WHILE
            break;

        case ST_BREAK:
            // I think i'll handle it in WHILE
            break;

        case ST_RETURN:
            // generate the return value expression, if any
            printf("\tRET\n");
            break;

        case ST_EXPRESSION:
            // generate expression using a DAG? as is?
            break;    
    }
}

void generate_function_code(func_declaration *func) {
    // stack frame, decoding of arguments, local data, etc.
    // where do we go from here?

    printf("%s:\n", func->func_name);
    printf("\tPUSH bp\n");
    printf("\tMOV bp, sp\n");

    // allocate stack space for func arguments and locals
    // find all var declarations in the subtree

    statement *stmt = func->stmts_list;
    while (stmt != NULL) {
        generate_statement_code(stmt);
        stmt = stmt->next;
    }

    printf("%s__exit__:\n", func->func_name);
    printf("\tPOP bp\n");
    printf("\tRET\n");
    printf("\n");
}

void generate_module_code(ast_module_node *module) {
    statement *stmt = module->statements_list;
    while (stmt != NULL) {
        if (stmt->stmt_type != ST_VAR_DECL) {
            error(NULL, 0, "only var declarations are supported in code generation");
        } else {
            // find size, allocate memory, get address (?)
        }
        stmt = stmt->next;
    }

    func_declaration *func = module->funcs_list;
    while (func != NULL) {
        generate_function_code(func);
        func = func->next;
    }

    printf("--- Generated code follows: ---\n");
    printf(".bss (uninitialized\n");
    generate_data_segment_assembly(false);
    printf(".data (initialized\n");
    generate_data_segment_assembly(true);
    printf(".text\n");
    generate_code_segment_assembly();
}

