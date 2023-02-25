#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../options.h"
#include "../err_handler.h"
#include "../elf/binary_program.h"
#include "binary_gen.h"


// see https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html#three-volume
// for encoding rules


struct code_segment {
    char *buffer;               // the binary bytes
    int buffer_capacity;        // size malloc'ed for *buffer
    unsigned long base_address; // for which the binary is created
    unsigned long curr_address; // current address, essentially base_address + offset
};

struct backfill_reminder {
    unsigned long address;   // where to put the final address
    char *label;             // which final address to put
};

struct label_association {
    char *label;             // which label 
    unsigned long address;   // corresponds to which offset;
};

struct working_memory {
    struct code_segment code;

    struct backfill_reminder **backfills; // array of pointers
    int backfills_capacity;               // array capacity
    int backfills_count;                  // how many we currently have

    struct label_association **labels; // array of pointers
    int labels_capacity;               // array capacity (realloc
    int labels_count;                  // how many we currently have

} wm;

static void init(unsigned long base_address) {
    wm.backfills_capacity = 10;
    wm.backfills = malloc(wm.backfills_capacity * sizeof(struct backfill_reminder *));

    wm.labels_capacity = 10;
    wm.labels = malloc(wm.labels_capacity * sizeof(struct label_association *));

    wm.code.buffer_capacity = 1024;
    wm.code.buffer = malloc(wm.code.buffer_capacity);
    wm.code.base_address = base_address;
    wm.code.curr_address = base_address;
}

static void add_label_association(char *label, unsigned address) {
    for (int i = 0; i < wm.labels_count; i++) {
        if (strcmp(wm.labels[i]->label, label) == 0) {
            error(NULL, 0, "label \"%s\" already defined!", label);
        }
    }
    struct label_association *la = malloc(sizeof(struct label_association));
    la->label = label;
    la->address = address;

    if (wm.labels_count == wm.labels_capacity) {
        wm.labels_capacity *= 2;
        wm.labels = realloc(wm.labels, wm.labels_capacity);
    }
    wm.labels[wm.labels_count++] = la;
}

static void add_backfill_reminder(unsigned address, char *label) {
    struct backfill_reminder *re = malloc(sizeof(struct backfill_reminder));
    re->address = address;
    re->label = label;

    if (wm.backfills_count == wm.backfills_capacity) {
        wm.backfills_capacity *= 2;
        wm.backfills = realloc(wm.backfills, wm.backfills_capacity);
    }
    wm.backfills[wm.backfills_count++] = re;
}

static void append_code_bytes(char *bytes, int length) {
    // just append at the current position
    unsigned offset = wm.code.curr_address - wm.code.base_address;
    if (offset + length >= wm.code.buffer_capacity) {
        wm.code.buffer_capacity *= 2;
        wm.code.buffer = realloc(wm.code.buffer, wm.code.buffer_capacity);
    }
    memcpy(&wm.code.buffer[offset], bytes, length);
    wm.code.curr_address += length;
}

static void replace_code_bytes(unsigned address, char *bytes, int length) {
    // replace if specific position, used for patching
    if (address < wm.code.base_address || address + length >= wm.code.curr_address) {
        error(NULL, 0, "attempted to patch at address 0x%x, while memory is from 0x%x to 0x%x",
            address, wm.code.base_address, wm.code.curr_address);
        return;
    }

    unsigned offset = address - wm.code.base_address;
    memcpy(wm.code.buffer + offset, bytes, length);
}

static void pass_1_encode_assembly(char *assembly) {
    char label[64];
    char opcode[64];
    char op1[64];
    char op2[64];
    char binary[16];
    int bin_len = 0;

    int line_no = 1;
    while (*assembly != '\0') {
        parse_asm_line(&assembly, label, opcode, op1, op2, line_no, 64);
        if (strlen(label) > 0)
            add_label_association(strdup(label), wm.code.curr_address);
        if (strlen(opcode) == 0)
            continue;
        if (!encode_asm(opcode, op1, op2, binary, &bin_len)) {
            error(NULL, 0, "error encoding \"%s %s %s\" into machine code", opcode, op1, op2);
            return;
        }
        // if an address resolution is needed, much mark it. e.g.
        // add_backfill_reminder(wm.code.curr_address + 2, op2);
        
        append_code_bytes(binary, bin_len);
        line_no++;
    }
}

static void pass_2_backfill_addresses(int address_size_in_bytes) {
    bool found;
    unsigned long resolved_address;

    for (int r = 0; r < wm.backfills_count; r++) {
        struct backfill_reminder *re = wm.backfills[r];

        // we need to find the label, O(n) it is...
        found = false;
        for (int l = 0; l < wm.labels_count; l++) {
            if (strcmp(wm.labels[l]->label, re->label) == 0) {
                found = true;
                resolved_address = wm.labels[l]->address;
                break;
            }
        }
        if (!found) {
            error(NULL, 0, "failed resolving label \"%s\" to an address", re->label);
            return;
        }
        replace_code_bytes(re->address, (char *)resolved_address, address_size_in_bytes);
    }
}

void generate_binary_code(char *assembly_code, binary_program **program) {
    // two passes, first generate assembly, marking jumps as open ended
    // then come back and fill in jumps final addresses
    init(0x800000);
    pass_1_encode_assembly(assembly_code);
    pass_2_backfill_addresses(8);

    (*program) = NULL;
}



#include "../x86_encoder/instruction.h"
#include "../x86_encoder/encoder.h"
#include "../x86_encoder/bin_buffer.h"
#include "../utils.h"
void perform_asm_test() {
    // we should really do this one: https://www.tutorialspoint.com/assembly_programming/assembly_system_calls.htm

    
    /*
        401000:	b8 34 12 00 00       	mov    $0x1234,%eax
        401005:	b8 01 00 00 00       	mov    $0x1,%eax
        40100a:	bb 08 00 00 00       	mov    $0x8,%ebx
        40100f:	cd 80                	int    $0x80
    */
    struct instruction list[4];

    list[0].opcode = OC_MOV;
    list[0].op1.type = OT_REGISTER;
    list[0].op1.value = REG_AX;
    list[0].op2.type = OT_IMMEDIATE_VALUE;
    list[0].op2.value = 0x1234;

    list[1].opcode = OC_MOV;
    list[1].op1.type = OT_REGISTER;
    list[1].op1.value = REG_AX;
    list[1].op2.type = OT_IMMEDIATE_VALUE;
    list[1].op2.value = 1;

    list[2].opcode = OC_MOV;
    list[2].op1.type = OT_REGISTER;
    list[2].op1.value = REG_BX;
    list[2].op2.type = OT_IMMEDIATE_VALUE;
    list[2].op2.value = 8;

    list[3].opcode = OC_INT;
    list[3].op1.type = OT_IMMEDIATE_VALUE;
    list[3].op1.value = 0x80;

    struct x86_encoder *encoder = new_x86_encoder(CPU_MODE_PROTECTED);
    struct bin_buffer *buffer = new_bin_buffer(10);
    for (int i = 0; i < sizeof(list)/sizeof(list[0]); i++) {
        encoder->encode(encoder, &list[i], buffer);
    }
    print_16_hex(buffer->data, buffer->length);
}
