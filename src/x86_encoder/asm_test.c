#include <stdio.h>
#include <string.h>
#include "instruction.h"
#include "encoder.h"
#include "bin_buffer.h"
#include "../utils.h"


static void test_push_pops();
static void test_movs();
static void test_instructions();
static void test_exiting_code();
static void verify_listing(char *title, struct instruction *list, int instr_count, char *expected, int expected_len);
static bool verify_single_instruction(struct instruction *instr, char *expected_bytes, int expected_len);


#define DECLARE_LIST() \
    struct instruction list[32];    \
    int list_len = 0;             \
    memset(&list, 0, sizeof(list));

#define ADD0(op)  \
    list[list_len].opcode = op; \
    list_len++;

#define ADD1_(op, op1type, op1value) \
    list[list_len].opcode = op; \
    list[list_len].op1.type = op1type; \
    list[list_len].op1.value = op1value; \
    list_len++;

#define ADD1o(op, op1type, op1value, op1offset) \
    list[list_len].opcode = op; \
    list[list_len].op1.type = op1type; \
    list[list_len].op1.value = op1value; \
    list[list_len].op1.offset = op1offset; \
    list_len++;

#define ADD1s(op, name) \
    list[list_len].opcode = op; \
    list[list_len].op1.type = OT_SYMBOL_MEM_ADDRESS; \
    list[list_len].op1.symbol_name = name; \
    list_len++;

#define ADD2(op, op1type, op1value, op2type, op2value) \
    list[list_len].opcode = op; \
    list[list_len].op1.type = op1type; \
    list[list_len].op1.value = op1value; \
    list[list_len].op2.type = op2type; \
    list[list_len].op2.value = op2value; \
    list_len++;

#define ADD2s(op, op1type, op1value, name) \
    list[list_len].opcode = op; \
    list[list_len].op1.type = op1type; \
    list[list_len].op1.value = op1value; \
    list[list_len].op2.type = OT_SYMBOL_MEM_ADDRESS; \
    list[list_len].op2.symbol_name = name; \
    list_len++;

#define ADDs2(op, name, op2type, op2value) \
    list[list_len].opcode = op; \
    list[list_len].op1.type = OT_SYMBOL_MEM_ADDRESS; \
    list[list_len].op1.symbol_name = name; \
    list[list_len].op2.type = op2type; \
    list[list_len].op2.value = op2value; \
    list_len++;


void perform_asm_test() {
    // test_push_pops();
    // test_movs();
    test_instructions();
}


static void test_push_pops() {
    struct instruction list[16];
    int list_len = 0;
    memset(&list, 0, sizeof(list));

    ADD1_(OC_PUSH, OT_REGISTER, REG_AX);
    ADD1o(OC_PUSH, OT_MEMORY_POINTED_BY_REG, REG_BX, +3);
    ADD1_(OC_PUSH, OT_IMMEDIATE, 0x1234);
    ADD1s(OC_PUSH, "nicholas");
    ADD1_(OC_POP, OT_REGISTER, REG_CX);
    ADD1o(OC_POP, OT_MEMORY_POINTED_BY_REG, REG_DX, +2);
    ADD1s(OC_POP, "nicholas");

    // using https://defuse.ca/online-x86-assembler.htm
    // 0:  50                      push   eax
    // 1:  ff 73 03                push   DWORD PTR [ebx+0x3]
    // 4:  68 34 12 00 00          push   0x1234
    // 9:  ff 35 00 00 00 00       push   DWORD PTR ds:0x0
    // f:  59                      pop    ecx
    // 10: 8f 42 02                pop    DWORD PTR [edx+0x2]
    // 13: 8f 05 00 00 00 00       pop    DWORD PTR ds:0x0

    u8 *expected = "\x50\xFF\x73\x03\x68\x34\x12\x00\x00\xFF\x35\x00\x00\x00\x00\x59\x8F\x42\x02\x8F\x05\x00\x00\x00\x00";
    int expected_len = 25;
    verify_listing("Push operations", list, list_len, expected, expected_len);
}

static void test_movs() {
    DECLARE_LIST();

    // using https://defuse.ca/online-x86-assembler.htm
    // MOV EAX, 1
    // MOV ECX, 1
    // MOV EDX, 1
    // MOV EAX, ESP
    // MOV EAX, EBP
    // MOV EAX, ESI
    // 0:  b8 01 00 00 00          mov    eax,0x1
    // 5:  b9 01 00 00 00          mov    ecx,0x1
    // a:  ba 01 00 00 00          mov    edx,0x1
    // f:  89 e0                   mov    eax,esp  (e0 = 11 100 000)
    // 11: 89 e8                   mov    eax,ebp  (e8 = 11 101 000)
    // 13: 89 f0                   mov    eax,esi  (f0 = 11 110 000)
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_IMMEDIATE, 1);
    ADD2(OC_MOV, OT_REGISTER, REG_CX, OT_IMMEDIATE, 1);
    ADD2(OC_MOV, OT_REGISTER, REG_DX, OT_IMMEDIATE, 1);
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_REGISTER, REG_SP);
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_REGISTER, REG_BP);
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_REGISTER, REG_SI);
    ADD0(OC_NOP);

    // MOV EAX, symbol
    // MOV symbol, EAX
    // 15: a1 00 00 00 00          mov    eax,ds:0x0
    // 1a: a3 00 00 00 00          mov    ds:0x0,eax
    ADD2s(OC_MOV, OT_REGISTER, REG_AX, "nicholas");
    ADDs2(OC_MOV, "nicholas", OT_REGISTER, REG_AX);

    // MOV EAX, [ECX]
    // MOV EAX, [EBP+4]
    // MOV EAX, [EBP-2]
    // 1f: 8b 01                   mov    eax,DWORD PTR [ecx]
    // 21: 8b 45 04                mov    eax,DWORD PTR [ebp+0x4]
    // 24: 8b 45 fe                mov    eax,DWORD PTR [ebp-0x2]
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_MEMORY_POINTED_BY_REG, REG_CX);
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_MEMORY_POINTED_BY_REG, REG_BP);
    list[list_len-1].op2.offset = +4;
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_MEMORY_POINTED_BY_REG, REG_BP);
    list[list_len-1].op2.offset = -2;

    // MOV DWORD PTR [ECX], EAX
    // MOV DWORD PTR [ECX], 1
    // MOV DWORD PTR [EBP+4], 1
    // MOV DWORD PTR [EBP-2], 1
    // 27: 89 01                   mov    DWORD PTR [ecx],eax
    // 29: c7 01 01 00 00 00       mov    DWORD PTR [ecx],0x1
    // 2f: c7 45 04 01 00 00 00    mov    DWORD PTR [ebp+0x4],0x1
    // 36: c7 45 fe 01 00 00 00    mov    DWORD PTR [ebp-0x2],0x1
    ADD2(OC_MOV, OT_MEMORY_POINTED_BY_REG, REG_CX, OT_REGISTER, REG_AX);
    ADD2(OC_MOV, OT_MEMORY_POINTED_BY_REG, REG_CX, OT_IMMEDIATE, 1);
    ADD2(OC_MOV, OT_MEMORY_POINTED_BY_REG, REG_BP, OT_IMMEDIATE, 1);
    list[list_len-1].op1.offset = +4;
    ADD2(OC_MOV, OT_MEMORY_POINTED_BY_REG, REG_BP, OT_IMMEDIATE, 1);
    list[list_len-1].op2.offset = -2;



    char *expected = "\xB8\x01\x00\x00\x00\xB9\x01\x00\x00\x00\xBA\x01\x00\x00\x00\x89\xE0\x89\xE8\x89\xF0\xA1\x00\x00\x00\x00\xA3\x00\x00\x00\x00\x8B\x01\x8B\x45\x04\x8B\x45\xFE\x89\x01\xC7\x01\x01\x00\x00\x00\xC7\x45\x04\x01\x00\x00\x00\xC7\x45\xFE\x01\x00\x00\x00";
    int expected_len = 244 / 4;

    verify_listing("MOVs", list, list_len, expected, expected_len);
}

#define VERIFY_INSTR0(oc, expect_bytes, expect_len)   \
    instr.opcode = oc; \
    instr.op1.type = OT_NONE; \
    instr.op2.type = OT_NONE; \
    if (!verify_single_instruction(&instr, expect_bytes, expect_len)) return;

#define VERIFY_INSTR1_IMMEDIATE(code, val, expect_bytes, expect_len) \
    instr.opcode = code; \
    instr.op1.type = OT_IMMEDIATE; \
    instr.op1.value = val; \
    instr.op2.type = OT_NONE; \
    if (!verify_single_instruction(&instr, expect_bytes, expect_len)) return;

static void test_instructions() {
    struct instruction instr;
    printf("Verifying instructions ");

    // no operands instruction
    VERIFY_INSTR0(OC_NOP, "\x90", 1);
    VERIFY_INSTR0(OC_RET, "\xC3", 1);

    // one operand instructions: immediate, register, mem<-reg, mem<-symbol
    VERIFY_INSTR1_IMMEDIATE(OC_INT, 0x21, "\xCD\x21", 2);
    VERIFY_INSTR1_IMMEDIATE(OC_INT, 0x80, "\xCD\x80", 2);
    VERIFY_INSTR1_IMMEDIATE(OC_PUSH, 0, "\x6A\x00", 2);
    VERIFY_INSTR1_IMMEDIATE(OC_PUSH, 1, "\x6A\x01", 2);
    VERIFY_INSTR1_IMMEDIATE(OC_PUSH, -1, "\x6A\xFF", 2);
    VERIFY_INSTR1_IMMEDIATE(OC_PUSH, 0x12345678, "\x68\x78\x56\x34\x12", 5);

    // VERIFY_INSTR1_IMMEDIATE(opcode, value, expect_bytes, expect_len);
    // VERIFY_INSTR1_REGISTER(opcode, regno, expect_bytes, expect_len);
    // VERIFY_INSTR1_MEMBYREG(opcode, regno, offset, expect_bytes, expect_len);
    // VERIFY_INSTR1_MEMBYSYM(opcode, symbol_name, expect_bytes, expect_len);

    // // two operands operations, target is a register
    // VERIFY_INSTR2_REG_REGISTER(opcode, target_regno, source_regno, expect_bytes, expect_len);
    // VERIFY_INSTR2_REG_IMMEDIATE(opcode, target_regno, value, expect_bytes, expect_len);
    // VERIFY_INSTR2_REG_MEMBYREG(opcode, target_regno, mem_regno, mem_offset, expect_bytes, expect_len);
    // VERIFY_INSTR2_REG_MEMBYSYM(opcode, target_regno, symbol_name, expect_bytes, expect_len);

    // // two operands operations, target is an address pointed by register +/- offset
    // VERIFY_INSTR2_MEMBYREG_REGISTER(opcode, mem_regno, mem_offset, source_regno, expect_bytes, expect_len);
    // instr.opcode = opcode; 
    // if (!verify_single_instruction(instr, expect_bytes, expect_len)) return;

    // VERIFY_INSTR2_MEMBYREG_IMMEDIATE(opcode, mem_regno, mem_offset, value, expect_bytes, expect_len);

    // // two operands operations, target is an address pointed by a symbol
    // VERIFY_INSTR2_MEMBYSYM_REGISTER(opcode, symbol_name, source_regno, expect_bytes, expect_len);
    // VERIFY_INSTR2_MEMBYSYM_IMMEDIATE(opcode, symbol_name, value, expect_bytes, expect_len);

    // if we got here, no test failed
    printf(" OK\n");
}

static void test_exiting_code() {
    // equivalent to "main() { exit(8); }"

    // 401005:	b8 01 00 00 00       	mov    $0x1,%eax
    // 40100a:	bb 08 00 00 00       	mov    $0x8,%ebx
    // 40100f:	cd 80                	int    $0x80
}

static void test_full_io() {
    // we should really do this one: https://www.tutorialspoint.com/assembly_programming/assembly_system_calls.htm
}





static bool verify_single_instruction(struct instruction *instr, char *expected_bytes, int expected_len) {
    char buff[128];
    instruction_to_string(instr, buff);

    struct x86_encoder *enc = new_x86_encoder(CPU_MODE_PROTECTED);
    if (!enc->encode(enc, instr)) {
        printf("\n");
        printf("  Could not encode instruction '%s'\n", buff);
        enc->free(enc);
        return false;
    }
    if (memcmp(enc->output->data, expected_bytes, expected_len) != 0) {
        printf("\n");
        printf("  Bad instruction encoding '%s'\n", buff);
        printf("  Expected:");
        for (int i = 0; i < expected_len; i++)
            printf(" %02x", (u8)expected_bytes[i]);
        printf("\n");
        printf("  Produced:");
        for (int i = 0; i < enc->output->length; i++)
            printf(" %02x", (u8)enc->output->data[i]);
        printf("\n");
        enc->free(enc);
        return false;
    }

    enc->free(enc);
    printf(".");
    return true;
}


static void verify_listing(char *title, struct instruction *list, int instr_count, char *expected, int expected_len) {
    bool encoded;
    char buff[128];

    printf("Verify listing '%s'...", title);
    
    struct x86_encoder *encoder = new_x86_encoder(CPU_MODE_PROTECTED);
    for (int i = 0; i < instr_count; i++) {
        encoded = encoder->encode(encoder, &list[i]);
        if (!encoded) {
            instruction_to_string(&list[i], buff);
            printf("Cannot encode expression: \'%s\'\n", buff);
            return;
        }
    }

    if (memcmp(encoder->output->data, expected, expected_len) == 0) {
        printf(" [OK]\n");
        return;
    }

    printf(" [FAILED]\n");

    printf("Assembly code listing:\n");
    for (int i = 0; i < instr_count; i++) {
        instruction_to_string(&list[i], buff);
        printf("\t%s\n", buff);
    }

    printf("Encoded machine code: (%d bytes)\n", encoder->output->length);
    print_16_hex(encoder->output->data, encoder->output->length);

    printf("Unresolved references:\n");
    printf("  Position  Symbol name\n");
    //        12345678  abcdef
    for (int i = 0; i < encoder->references->length; i++) {
        struct reference *r = &encoder->references->references[i];
        printf("  %8ld  %s\n", r->position, r->name);
    }

    printf("Expected: ");
    for (int i = 0; i < expected_len; i++)
        printf(" %02x", (unsigned char)expected[i]);
    printf("\n");
    
    printf("Produced: ");
    for (int i = 0; i < encoder->output->length; i++)
        printf(" %02x", (unsigned char)encoder->output->data[i]);
    printf("\n");
}
