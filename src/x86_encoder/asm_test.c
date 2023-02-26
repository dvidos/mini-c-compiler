#include <stdio.h>
#include <string.h>
#include "instruction.h"
#include "encoder.h"
#include "bin_buffer.h"
#include "../utils.h"


static void test_listing1();
static void test_listing2();
static void test_listing3();
static void verify_listing(char *title, struct instruction *list, int instr_count, char *expected, int expected_len);


void perform_asm_test() {
    test_listing1();
    test_listing2();
    test_listing3();
}


static void test_listing1() {
    struct instruction list[16];
    int count = 0;
    memset(&list, 0, sizeof(list));

    list[count].opcode = OC_PUSH;
    list[count].op1.type = OT_REGISTER;
    list[count].op1.value = REG_AX;
    count++;

    list[count].opcode = OC_PUSH;
    list[count].op1.type = OT_MEMORY_POINTED_BY_REG;
    list[count].op1.value = REG_BX;
    count++;

    list[count].opcode = OC_PUSH;
    list[count].op1.type = OT_IMMEDIATE;
    list[count].op1.value = 0x1234;
    count++;

    list[count].opcode = OC_PUSH;
    list[count].op1.type = OT_MEMORY_ADDRESS_OF_SYMBOL;
    list[count].op1.symbol_name = "nicholas";
    count++;

    list[count].opcode = OC_POP;
    list[count].op1.type = OT_REGISTER;
    list[count].op1.value = REG_CX;
    count++;

    list[count].opcode = OC_POP;
    list[count].op1.type = OT_MEMORY_POINTED_BY_REG;
    list[count].op1.value = REG_DX;
    count++;

    list[count].opcode = OC_POP;
    list[count].op1.type = OT_MEMORY_ADDRESS_OF_SYMBOL;
    list[count].op1.symbol_name = "nicholas";
    count++;

    // 0:  50                      push   eax
    // 1:  ff 73 03                push   DWORD PTR [ebx+0x3]
    // 4:  68 34 12 00 00          push   0x1234
    // 9:  ff 35 00 00 00 00       push   DWORD PTR ds:0x0
    // f:  59                      pop    ecx
    // 10: 8f 42 02                pop    DWORD PTR [edx+0x2]
    // 13: 8f 05 00 00 00 00       pop    DWORD PTR ds:0x0


    // using https://defuse.ca/online-x86-assembler.htm
    u8 *expected = "\x50\xFF\x73\x03\x68\x34\x12\x00\x00\xFF\x35\x00\x00\x00\x00\x59\x8F\x42\x02\x8F\x05\x00\x00\x00\x00";
    int expected_len = 25;
    verify_listing("Push operations", list, count, expected, expected_len);
}


static void test_listing2() {
    // equivalent to "main() { exit(8); }"

    // 401005:	b8 01 00 00 00       	mov    $0x1,%eax
    // 40100a:	bb 08 00 00 00       	mov    $0x8,%ebx
    // 40100f:	cd 80                	int    $0x80

    // list[1].opcode = OC_MOV;
    // list[1].op1.type = OT_REGISTER;
    // list[1].op1.value = REG_AX;
    // list[1].op2.type = OT_IMMEDIATE;
    // list[1].op2.value = 1;

    // list[2].opcode = OC_MOV;
    // list[2].op1.type = OT_REGISTER;
    // list[2].op1.value = REG_BX;
    // list[2].op2.type = OT_IMMEDIATE;
    // list[2].op2.value = 8;

    // list[3].opcode = OC_INT;
    // list[3].op1.type = OT_IMMEDIATE;
    // list[3].op1.value = 0x80;
}

static void test_listing3() {
    // we should really do this one: https://www.tutorialspoint.com/assembly_programming/assembly_system_calls.htm
}

static void verify_listing(char *title, struct instruction *list, int instr_count, char *expected, int expected_len) {

    struct x86_encoder *encoder = new_x86_encoder(CPU_MODE_PROTECTED);
    for (int i = 0; i < instr_count; i++) {
        encoder->encode(encoder, &list[i]);
    }

    printf("%s\n", title);

    printf("Assembly code listing:\n");
    for (int i = 0; i < instr_count; i++)
        print_instruction("\t", &list[i]);

    printf("Encoded machine code: (%d bytes)\n", encoder->output->length);
    print_16_hex(encoder->output->data, encoder->output->length);

    printf("Unresolved references:\n");
    printf("  Position  Symbol name\n");
    //        12345678  abcdef
    for (int i = 0; i < encoder->references->count; i++) {
        struct reference *r = &encoder->references->references[i];
        printf("  %8ld  %s\n", r->position, r->name);
    }

    if (memcmp(encoder->output->data, expected, expected_len) == 0)
        printf("[OK]\n");
    else {
        printf("[FAILED] Was expecting: ");
        for (int i = 0; i < expected_len; i++)
            printf(" %02x", (unsigned char)expected[i]);
        printf("\n");
        
        printf("         Gotten       : ");
        for (int i = 0; i < encoder->output->length; i++)
            printf(" %02x", (unsigned char)encoder->output->data[i]);
        printf("\n");
    }
}
